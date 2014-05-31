#!/usr/bin/env python
from zmq_client import ZMQClient

client = ZMQClient(port=5555)
client.receive_and_save_messages()
