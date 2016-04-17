#include <apphook.h>
#include <libtest/testutils.h>

#include "modules/zmq/zmq-module.h"

void
test_source_methods(){
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);

  zmq_sd_set_address((LogDriver *) self, "example_address");
  zmq_sd_set_port((LogDriver *) self, 42);
  assert_string(get_address(self), "tcp://example_address:42", "Failed to set zmq address", NULL);
  assert_string(get_persist_name(self), "zmq_source:example_address:42", "Given persist name is not okay", NULL);
  assert_false(create_zmq_context(self), "Successfully created new zmq context with fake data which is unexpected");

  zmq_sd_set_address((LogDriver *) self, "localhost");
  zmq_sd_set_port((LogDriver *) self, 65530);
  assert_string(get_address(self), "tcp://localhost:65530", "Failed to set zmq address", NULL);
  assert_true(create_zmq_context(self), "Failed to create zmq context");
}

void
test_destination_methods()
{
  ZMQDestDriver *self = g_new0(ZMQDestDriver, 1);
  zmq_dd_set_port((LogDriver *)self, 2222);
  assert_true(zmq_dd_connect(self), "Failed to connect with default parameters!", NULL);
  assert_string(self->port, "2222", "Failed to set port in zmq-destination", NULL);
}


int main()
{
  app_startup();

  test_source_methods();
  test_destination_methods();

  app_shutdown();
  return 0;
}
