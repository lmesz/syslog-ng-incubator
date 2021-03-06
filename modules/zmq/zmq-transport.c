/*
 * Copyright (c) 2002-2012 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 1998-2012 Balázs Scheidler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 */

#include "zmq-transport.h"
#include "zmq-source.h"
#include <zmq.h>

typedef struct _LogTransportZMQ
{
  LogTransport super;
  void* soc;
} LogTransportZMQ;

static gssize
log_transport_zmq_read_method(LogTransport *s, gpointer buf, gsize buflen, GSockAddr **sa)
{
  LogTransportZMQ *self = (LogTransportZMQ *) s;
  return zmq_recv(self->soc, buf, buflen, ZMQ_DONTWAIT);
}

static gssize
log_transport_zmq_write_method(LogTransport *s, const gpointer buf, gsize buflen)
{
  LogTransportZMQ *self = (LogTransportZMQ *) s;
  return zmq_send (self->soc, buf, buflen, ZMQ_DONTWAIT);
}

static void
log_transport_zmq_free_method(LogTransport *s)
{
  LogTransportZMQ *self = (LogTransportZMQ *) s;
}

static inline gchar*
get_bind_address(ZMQSourceDriver *self)
{
  return g_strdup_printf("tcp://%s:%d", self->address, self->port);
}

LogTransport *
log_transport_zmq_new(ZMQSourceDriver* src_driver)
{
  LogTransportZMQ *self = g_new0(LogTransportZMQ, 1);
  int fd;
  size_t fd_size = sizeof(fd);
  self->soc = zmq_socket(src_driver->zmq_reader_context->zmq_context, ZMQ_PULL);

  zmq_bind(self->soc, get_bind_address(src_driver));

  zmq_getsockopt(self->soc, ZMQ_FD, &fd, &fd_size);
  log_transport_init_method(&self->super, fd);

  self->super.read = log_transport_zmq_read_method;
  self->super.write = log_transport_zmq_write_method;
  self->super.free_fn = log_transport_zmq_free_method;

  return &self->super;
}
