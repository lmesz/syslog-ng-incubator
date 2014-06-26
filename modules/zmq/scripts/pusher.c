#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"


int main(int argc, char* argv[])
{
    void *context = zmq_ctx_new();
    void *sink = zmq_socket(context, ZMQ_PUSH);

    zmq_connect (sink, "tcp://localhost:5558");
    s_send(sink, "Polipokelore!");

    zmq_close(sink);
    zmq_ctx_destroy(context);
    return 0;
}
