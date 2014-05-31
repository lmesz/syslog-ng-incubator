#!/usr/bin/env python
from zmq_server import ZMQServer

server = ZMQServer(port=5555)
server.send_messages()
