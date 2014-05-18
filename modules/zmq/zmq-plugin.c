/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Laszlo Meszaros <lmesz@balabit.hu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include <zmq.h>

#include "zmq-plugin.h"
#include "zmq-parser.h"
#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"
#include "logthrdestdrv.h"

typedef struct
{
  LogThrDestDriver super;
  gchar *port;

  GlobalConfig *cfg;

  LogTemplateOptions template_options;
  LogTemplate *template;

  void *context;
  void *publisher;

  gint32 seq_num;
} ZMQDriver;

void zmq_dd_set_port(LogDriver *destination, gchar *port);
void zmq_dd_set_template(LogDriver *destination, gchar *template);

/*
 * Configuration
 */
void
zmq_dd_set_port(LogDriver *destination, gchar *port)
{
    ZMQDriver *self = (ZMQDriver *)destination;
    self->port = g_strdup(port);
}

void
zmq_dd_set_template(LogDriver *destination, gchar *template)
{
    ZMQDriver *self = (ZMQDriver *)destination;
    self->template = log_template_new(self->cfg, NULL);
    log_template_compile(self->template, template, NULL);
}


LogTemplateOptions *
zmq_dd_get_template_options(LogDriver *d)
{
  ZMQDriver *self = (ZMQDriver *)d;
  return &self->template_options;
}

/*
 * Utilities
 */

static gchar *
zmq_dd_format_stats_instance(LogThrDestDriver *d)
{
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name), "zmq()");
  return persist_name;
}

static gchar *
zmq_dd_format_persist_name(LogThrDestDriver *d)
{
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name), "zmq()");
  return persist_name;
}

static gboolean
zmq_dd_connect(ZMQDriver *self, gboolean reconnect)
{
  gboolean bind_result = TRUE;
  self->context = zmq_ctx_new();
  self->publisher = zmq_socket(self->context, ZMQ_PUB);

  gchar *connection_string = g_strconcat("tcp://*:", self->port, NULL);

  if (zmq_bind(self->publisher, connection_string) == 0)
  {
    msg_verbose("Succesfully bind!", evt_tag_str("Port", self->port), NULL);
    bind_result = TRUE;
  }
  else
  {
    msg_verbose("Failed to bind!", evt_tag_str("Port", self->port), NULL);
    bind_result = FALSE;
  }

  g_free(connection_string);
  return bind_result;
}

static void
zmq_dd_disconnect(LogThrDestDriver *destination)
{
  ZMQDriver *self = (ZMQDriver *)destination;
  zmq_close(self->publisher);
  zmq_ctx_destroy(self->context);
}

/*
 * Worker thread
 */

static gboolean
zmq_worker_insert(LogThrDestDriver *destination)
{
  ZMQDriver *self = (ZMQDriver *)destination;
  gboolean success = TRUE;
  LogMessage *msg;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
  GString *result = g_string_new("");

  success = log_queue_pop_head(self->super.queue, &msg, &path_options, FALSE, FALSE);

  if(!success)
    return TRUE;

  msg_set_context(msg);

  log_template_format(self->template, msg, NULL, LTZ_LOCAL, self->seq_num, NULL, result);

  msg_set_context(NULL);

  if (!zmq_send (self->publisher, result->str, result->len, 0))
  {
      msg_error("Failed to add message to zmq queue!", evt_tag_errno("errno", errno), NULL);
      success = FALSE;
  }

  g_string_free(result, TRUE);
  return success;
}

static void
zmq_worker_thread_init(LogThrDestDriver *destination)
{
  ZMQDriver *self = (ZMQDriver *)destination;

  msg_debug("Worker thread started",
            evt_tag_str("driver", self->super.super.super.id),
            NULL);

  zmq_dd_connect(self, FALSE);
}

static void
zmq_worker_thread_deinit(LogThrDestDriver *destination)
{
}

/*
 * Main thread
 */

static gboolean
zmq_dd_init(LogPipe *destination)
{
  ZMQDriver *self = (ZMQDriver *)destination;
  GlobalConfig *cfg = log_pipe_get_config(destination);

  self->cfg = cfg;

  log_template_options_init(&self->template_options, self->cfg);

  if (!log_dest_driver_init_method(destination))
    return FALSE;

  msg_verbose("Initializing ZeroMQ destination",
              evt_tag_str("driver", self->super.super.super.id),
              NULL);

  return log_threaded_dest_driver_start(destination);
}

static void
zmq_dd_free(LogPipe *destination)
{
  log_threaded_dest_driver_free(destination);
}

/*
 * Plugin glue.
 */

LogDriver *
zmq_dd_new(GlobalConfig *cfg)
{
  ZMQDriver *self = g_new0(ZMQDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super);
  self->super.super.super.super.init = zmq_dd_init;
  self->super.super.super.super.free_fn = zmq_dd_free;

  self->super.worker.thread_init = zmq_worker_thread_init;
  self->super.worker.thread_deinit = zmq_worker_thread_deinit;
  self->super.worker.disconnect = zmq_dd_disconnect;
  self->super.worker.insert = zmq_worker_insert;

  self->super.format.stats_instance = zmq_dd_format_stats_instance;
  self->super.format.persist_name = zmq_dd_format_persist_name;
  self->super.stats_source = 200;

  zmq_dd_set_port((LogDriver *) self, "5556");
  zmq_dd_set_template((LogDriver *) self, "${MESSAGE}");

  init_sequence_number(&self->seq_num);

  return (LogDriver *)self;
}

extern CfgParser zmq_dd_parser;

static Plugin zmq_plugin =
{
  .type = LL_CONTEXT_DESTINATION,
  .name = "zmq",
  .parser = &zmq_parser,
};

gboolean
zmq_module_init(GlobalConfig *cfg, CfgArgs *args)
{
  plugin_register(cfg, &zmq_plugin, 1);

  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "zmq",
  .version = VERSION,
  .description = "The zmq module provides ZeroMQ destination support for syslog-ng.",
  .core_revision = "Dummy Revision",
  .plugins = &zmq_plugin,
  .plugins_len = 1,
};
