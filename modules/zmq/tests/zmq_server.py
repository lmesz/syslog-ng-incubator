import zmq
import time

class ZMQServer(object):
    def __init__(self, port):
        self.ctx = zmq.Context()
        self.socket = self.ctx.socket(zmq.PUB)
        self.socket.bind("tcp://*:%d" % port)
        time.sleep(1)

    def send_messages(self, number_of_expected_send_msgs=1):
        for i in range(0, number_of_expected_send_msgs):
            message = "Message %d" % i
            self.socket.send(message)
            print "Sent message is: %s" % message
            time.sleep(1)

    def __del__(self):
        self.socket.close()
        self.ctx.term()
