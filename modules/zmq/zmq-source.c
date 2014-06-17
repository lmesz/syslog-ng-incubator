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
#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"

static void
zmq_sd_accept(gpointer s)
{
    /* transport and proto handling otherwise it spins */
    ZMQSourceDriver *self = (ZMQSourceDriver *) s;
    msg_verbose("MSG RECEIVED!!!!!", evt_tag_int("FD", self->fd), NULL);
}

static void
zmq_sd_start_watches(ZMQSourceDriver *self)
{
  IV_FD_INIT(&self->listen_fd);
  self->listen_fd.fd = self->fd;
  self->listen_fd.cookie = self;
  self->listen_fd.handler_in = zmq_sd_accept;
  iv_fd_register(&self->listen_fd);
}

static gboolean
zmq_sd_init(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(s);
  self->proto_factory = log_proto_server_get_factory(cfg, "zmq");
  log_reader_options_init(&self->reader_options, cfg, &self->super.super.group);

  /* TODO: Separate and ...*/
  int64_t fd = 0;
  size_t fd_size = sizeof (fd);
  void *context = zmq_ctx_new();
  void *soc = zmq_socket(context, ZMQ_PULL);

  if (soc)
    {
      zmq_getsockopt(soc, ZMQ_FD, &fd, &fd_size);
      self->fd = fd;
    }
  zmq_bind (soc, "tcp://*:5558");
  zmq_sd_start_watches(self);

  return log_src_driver_init_method(s);
}

static void
zmq_sd_queue(LogPipe *s, LogMessage *msg, const LogPathOptions *path_options, gpointer user_data)
{
  log_pipe_forward_msg(s, msg, path_options);
}

static gboolean
zmq_sd_deinit(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  return TRUE;
}

static void
zmq_sd_notify(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
}

static void
zmq_sd_free(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
}

LogDriver *
zmq_sd_new(GlobalConfig *cfg)
{
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);
  log_src_driver_init_instance(&self->super);
  self->super.super.super.init = zmq_sd_init;
  self->super.super.super.queue = zmq_sd_queue;
  self->super.super.super.deinit = zmq_sd_deinit;
  self->super.super.super.notify = zmq_sd_notify;
  self->super.super.super.free_fn = zmq_sd_free;
  log_reader_options_defaults(&self->reader_options);

  self->reader_options.parse_options.flags |= LP_LOCAL;
  return &self->super.super;
}
