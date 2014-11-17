/*
 * Copyright (c) 2014 Laszlo Meszaros <lacienator@gmail.com>
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

#include <stdlib.h>
#include <zlib.h>

#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats/stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"
#include "logthrdestdrv.h"

#ifndef SCS_WEBSOCKET
#define SCS_WEBSOCKET 0
#endif

typedef struct
{
  LogThrDestDriver super;

  gchar *host;
  gint port;

  LogTemplateOptions template_options;

} WebsocketDriver;

static gchar *
websocket_dd_format_stats_instance(LogThrDestDriver *d)
{
  return "";
}

static gchar *
websocket_dd_format_persist_name(LogThrDestDriver *d)
{
  return "";
}

static void
websocket_dd_disconnect(LogThrDestDriver *destination)
{
  msg_info("dd disconnect!", NULL);
}

static worker_insert_result_t
websocket_worker_insert(LogThrDestDriver *destination, LogMessage *msg)
{
  return WORKER_INSERT_RESULT_SUCCESS;
}
static void
websocket_worker_thread_init(LogThrDestDriver *destination)
{
  msg_info("Worker insert!", NULL);
}

static void
websocket_worker_thread_deinit(LogThrDestDriver *destination)
{
  msg_info("Worker deinit!", NULL);
}

gboolean
websocket_dd_init(LogPipe *destination)
{
  return TRUE;
}

static void
websocket_dd_free(LogPipe *destination)
{
  log_threaded_dest_driver_free(destination);
}

LogDriver *
websocket_dd_new(GlobalConfig *cfg)
{
  WebsocketDriver *self = g_new0(WebsocketDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super, cfg);
  self->super.super.super.super.init = websocket_dd_init;
  self->super.super.super.super.free_fn = websocket_dd_free;

  self->super.worker.thread_init = websocket_worker_thread_init;
  self->super.worker.thread_deinit = websocket_worker_thread_deinit;
  self->super.worker.disconnect = websocket_dd_disconnect;
  self->super.worker.insert = websocket_worker_insert;

  self->super.format.stats_instance = websocket_dd_format_stats_instance;
  self->super.format.persist_name = websocket_dd_format_persist_name;
  self->super.stats_source = SCS_WEBSOCKET;

  websocket_dd_set_host((LogDriver *)self, "localhost");
  websocket_dd_set_port((LogDriver *)self, 9998);

  log_template_options_defaults(&self->template_options);

  return (LogDriver *)self;
}

