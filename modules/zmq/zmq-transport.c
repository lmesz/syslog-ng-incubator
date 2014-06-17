#include "logtransport.h"

typedef struct _LogTransportZMQ LogTransportZMQ;
struct _LogTransportZMQ
{
  LogTransport super;
};

static gssize
log_transport_zmq_socket_read_method(LogTransport *s, gpointer buf, gsize buflen, GSockAddr **sa)
{
    return 0;
}

static gssize
log_transport_zmq_socket_write_method(LogTransport *s, const gpointer buf, gsize buflen)
{
    return 0;
}

static void
log_transport_zmq_socket_free_method(LogTransport *s)
{
}

LogTransport *
log_transport_zmq_socket_new(gint fd)
{
  LogTransportSocket *self = g_new0(LogTransportZMQ, 1);

  log_transport_init_method(&self->super, fd);
  self->super.read = log_transport_zmq_socket_read_method;
  self->super.write = log_transport_zmq_socket_write_method;
  self->super.free_fn = log_transport_zmq_socket_free_method;
  return &self->super;
}

