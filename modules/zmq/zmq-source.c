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

#include "zmq-source.h"
#include "zmq-parser.h"
#include "zmq-transport.h"

#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"
#include "poll-fd-events.h"
#include "logproto/logproto-text-server.h"

void
zmq_sd_set_address(LogDriver *source, gchar *address)
{
    ZMQSourceDriver *self = (ZMQSourceDriver *)source;
    g_free(self->address);
    self->address = g_strdup(address);
}

void
zmq_sd_set_port(LogDriver *source, gint port)
{
    ZMQSourceDriver *self = (ZMQSourceDriver *)source;
    self->port = port;
}

static void
create_reader(LogPipe *s)
{
  ZMQSourceDriver* self = (ZMQSourceDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  log_reader_options_init(&self->reader_options, cfg, "zmq");

  LogTransport* transport = log_transport_zmq_new(self->soc);
  PollEvents* poll_events = poll_fd_events_new(transport->fd);
  LogProtoServerOptions* proto_options = &self->reader_options.proto_options.super;
  LogProtoServer* proto = log_proto_text_server_new(transport, proto_options);

  self->reader = log_reader_new();
  log_reader_reopen(self->reader, proto, poll_events);

  log_reader_set_options(self->reader,
                             s,
                             &self->reader_options,
                             STATS_LEVEL1,
                             SCS_ZMQ,
                             self->super.super.id,
                             "test");
  log_pipe_append((LogPipe *) self->reader, s);
}

static gboolean
zmq_socket_init(ZMQSourceDriver *self)
{
  self->zmq_context = zmq_ctx_new();
  self->soc = zmq_socket(self->zmq_context, ZMQ_PULL);
  gboolean result = TRUE;
  gchar* location = g_strdup_printf("tcp://%s:%d", self->address, self->port);

  if (zmq_bind(self->soc, location) != 0)
  {
      msg_error("Failed to bind!", evt_tag_str("Bind address", location), NULL);
      result = FALSE;
      goto exit;
  }

exit:
  g_free(location);
  return result;
}

static gboolean
zmq_sd_init(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;

  if (!log_src_driver_init_method(s))
  {
    msg_error("Failed to initialize source driver!", NULL);
    return FALSE;
  }

  if (!zmq_socket_init(self))
      return FALSE;

  create_reader(s);

  if (!log_pipe_init((LogPipe *) self->reader, NULL))
  {
    msg_error("Error initializing log_reader", NULL);
    log_pipe_unref((LogPipe *) self->reader);
    self->reader = NULL;
    return FALSE;
  }
  return TRUE;
}

static gboolean
zmq_sd_deinit(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;

  if (self->reader)
  {
    log_pipe_deinit((LogPipe *) self->reader);
    log_pipe_unref((LogPipe *) self->reader);
    self->reader = NULL;
  }

  zmq_ctx_destroy(self->zmq_context);
  log_src_driver_deinit_method(s);
  return TRUE;
}

static void
zmq_sd_notify(LogPipe *s, gint notify_code, gpointer user_data)
{
  switch (notify_code)
    {
    case NC_CLOSE:
    case NC_READ_ERROR:
      {
        zmq_sd_deinit(s);
        break;
      }
    }
}

static void
zmq_sd_free(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  g_assert(!self->reader);
  log_reader_options_destroy(&self->reader_options);
  log_src_driver_free(s);
}

LogDriver *
zmq_sd_new()
{
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);
  log_src_driver_init_instance(&self->super);

  self->super.super.super.init = zmq_sd_init;
  self->super.super.super.deinit = zmq_sd_deinit;
  self->super.super.super.notify = zmq_sd_notify;
  self->super.super.super.free_fn = zmq_sd_free;
  log_reader_options_defaults(&self->reader_options);

  zmq_sd_set_address((LogDriver *) self, "*");
  zmq_sd_set_port((LogDriver *) self, 5558);

  return &self->super.super;
}
