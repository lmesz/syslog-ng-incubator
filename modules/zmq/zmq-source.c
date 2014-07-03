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


static void
zmq_sd_ack_message(LogMessage *msg, gpointer user_data)
{
    ZMQSourceDriver *self = (ZMQSourceDriver *) user_data;
    msg_verbose("LogMessage Acked!!!", NULL);
    log_msg_unref(msg);
    log_pipe_unref(&self->super.super.super);
}


static void
zmq_sd_accept(gpointer s)
{
    ZMQSourceDriver *self = (ZMQSourceDriver *) s;
    LogTransport* zmq_trans = log_transport_zmq_new(self->soc, self->fd);

    iv_fd_unregister(&self->listen_fd);

    while(TRUE)
    {
      int rc = zmq_recv(self->soc, self->buffer->str, self->buffer->allocated_len, ZMQ_DONTWAIT);

      if (rc < 0)
        {
          if (errno != EAGAIN && errno != EINTR)
            {
              msg_error("Something evil happened!\n", evt_tag_errno("Error", errno), NULL);
            }
          fprintf(stderr, "EAGAIN!!!!\n");
          goto exit;
        }

      self->buffer->len = rc;
      LogMessage *msg = log_msg_new(self->buffer->str, self->buffer->len, NULL, &self->reader_options.parse_options);
      log_msg_refcache_start_producer(msg);
      LogPathOptions po = LOG_PATH_OPTIONS_INIT;
      log_msg_ref(msg);
      log_msg_add_ack(msg, &po);
      msg->ack_func = zmq_sd_ack_message;
      msg->ack_userdata = log_pipe_ref(&self->super.super.super);

      log_pipe_queue(s, msg, &po);

      log_msg_refcache_stop();
    }
exit:
  iv_fd_register(&self->listen_fd);
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

  log_reader_options_init(&self->reader_options, cfg, "zmq");

  int64_t fd = 0;
  size_t fd_size = sizeof (fd);

  self->zmq_context = zmq_ctx_new();
  void *soc = zmq_socket(self->zmq_context, ZMQ_PULL);

  zmq_getsockopt(soc, ZMQ_FD, &fd, &fd_size);
  self->soc = soc;
  self->fd = fd;
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
  zmq_close(self->soc);
  zmq_ctx_destroy(self->zmq_context);
  return TRUE;
}

static void
zmq_sd_free(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  g_string_free(self->buffer, TRUE);
}

LogDriver *
zmq_sd_new(GlobalConfig *cfg)
{
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);
  log_src_driver_init_instance(&self->super);

  self->super.super.super.init = zmq_sd_init;
  self->super.super.super.queue = zmq_sd_queue;
  self->super.super.super.deinit = zmq_sd_deinit;
  self->super.super.super.free_fn = zmq_sd_free;
  log_reader_options_defaults(&self->reader_options);
  self->buffer = g_string_sized_new(cfg->log_msg_size + 1);

  return &self->super.super;
}
