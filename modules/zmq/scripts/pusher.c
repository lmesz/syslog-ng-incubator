#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"


int main(int argc, char* argv[])
{
    int number_of_messages = 1;
    char buffer[200] = {0};

    if (argc > 1)
    {
        number_of_messages = atoi(argv[1]);
    }
    fprintf(stderr, "Send %d messages\n", number_of_messages);

    void *context = zmq_ctx_new();
    void *sink = zmq_socket(context, ZMQ_PUSH);

    zmq_setsockopt(sink, ZMQ_SNDHWM, &hwm_size, sizeof(int));

    zmq_connect (sink, "tcp://localhost:5558");
    int i = 0;
    int rc = 0;
    for (i = 0; i < number_of_messages; i++)
      {
        rc = sprintf(buffer, "Polipokelore! %d\n", i);
        rc = zmq_send(sink, buffer, rc, 0);
        //printf("Sent message[%d]: %s with result: %d\n", i, buffer, rc);
      }
    zmq_close(sink);
    zmq_ctx_destroy(context);
    return 0;
}
