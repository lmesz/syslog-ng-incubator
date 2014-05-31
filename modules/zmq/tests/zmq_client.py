import threading
import zmq

class ZMQClient(threading.Thread):
    def __init__(self, port):
        super(ZMQClient, self).__init__()
        self.port = port
        self.result_filename = "test_result"

        self.ctx = zmq.Context()
        self.socket = self.ctx.socket(zmq.SUB)
        self.socket.connect("tcp://*:%d" % port)
        self.socket.setsockopt(zmq.SUBSCRIBE, "")

        print("Port %d is ready to receive messages!" % port)

    def run(self):
        self.receive_and_save_messages()

    def receive_and_save_messages(self):
        print("Waiting for receive message on port %d" % self.port)
        self.__empty_file(self.result_filename)
        msg_in_bytes = self.socket.recv()
        self.__add_content_to_result(content=msg_in_bytes)

    def __empty_file(self, filename):
        return open(filename, "w").close()

    def __add_content_to_result(self, content):
        with open(self.result_filename, "a") as f:
            f.write(content)

    def __del__(self):
        self.socket.close()
        self.ctx.term()
