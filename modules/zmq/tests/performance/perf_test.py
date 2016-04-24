#!/usr/bin/env python
import os
import sys
import subprocess
import zmq
import timeit
import time
import socket
import threading

conf_source = """
@version: 3.7
@include "scl.conf"

source zmq {
    SOURCE_TYPE
};

destination f {
    file("test_result");
};

log {
    source(zmq);
    destination(f);
};

"""

conf_destination = """
@version: 3.7
@include "scl.conf"

source zmq {
    SOURCE
};

destination f {
    DESTINATION
};

log {
    source(zmq);
    destination(f);
};

"""

if len(sys.argv) < 2:
    print("Not enought arguments!")
    sys.exit()

path = os.path.abspath(os.path.dirname(__file__))
NUMBER_OF_MESSAGES = int(sys.argv[1])
number_of_lines = 0

class Receiver(threading.Thread):
    def __init__(self, threadID, name, counter):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.counter = counter

    def run(self):
        self.receive_zmq_message()

    def receive_zmq_message(self):
        global number_of_lines
        res = 0
        ctx = zmq.Context()
        sock = ctx.socket(zmq.PULL)
        sock.connect("tcp://0.0.0.0:5556")
        for i in xrange(1, NUMBER_OF_MESSAGES):
            sock.recv()
            number_of_lines += 1
        ctx.destroy()

if not os.environ.has_key('SYSLOG_NG'):
    print("Please set syslog-ng location else test will not work!")
    sys.exit(1)

if len(sys.argv) < 2:
    print("Number of messages is missing. Please add it.")
    sys.exit()

def main():
    for i in xrange(1,4):
        print("%d. test" % i)
        measure_source("zmq")
        measure_source("tcp")
        print("###################################\n")

    for i in xrange(1,4):
        print("%d. test" % i)
        measure_destination("zmq")
        measure_destination("tcp")
        print("###################################\n")


def measure_source(source_type):
    remove_if_exists(path + '/test_result')
    remove_if_exists(path + '/slng.conf')
    remove_if_exists(path + '/persist')

    with open(path + '/slng.conf', 'w') as slng_conf:
        if source_type is "zmq":
            slng_conf.write(conf_source.replace("SOURCE_TYPE", "zmq();"))
        else:
            slng_conf.write(conf_source.replace("SOURCE_TYPE", "tcp(port(6644));"))

    p = subprocess.Popen([os.environ['SYSLOG_NG'], '-F', '-f', path + '/slng.conf', '-R', path + '/persist'])
    time.sleep(2)

    start_time = timeit.default_timer()
    if (source_type is "zmq"):
        send_zmq_messages()
    else:
        send_tcp_messages()

    while True:
        if arrived_line() >= NUMBER_OF_MESSAGES:
            print("Execution time of %s source is %f sec" % (source_type, timeit.default_timer() - start_time))
            break

    p.kill()

def remove_if_exists(file_name):
    if os.path.isfile(file_name):
        os.unlink(file_name)

def send_zmq_messages():
    ctx = zmq.Context()
    soc = ctx.socket(zmq.PUSH)
    soc.bind("tcp://127.0.0.1:5558")
    for i in xrange(NUMBER_OF_MESSAGES):
        soc.send("Test message!!!!!\n")
    ctx.destroy()

def send_tcp_messages():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("0.0.0.0", 6644))
    for i in xrange(NUMBER_OF_MESSAGES):
        s.send("Test message!!!!!\n")
    s.close()

def arrived_line():
    with open(path + "/test_result", "r") as res:
        return len(res.readlines())

def measure_destination(destination_type):
    tr = Receiver(1, "Just an ID", 1)
    tr.start()

    remove_if_exists(path + '/slng-test.conf')
    remove_if_exists(path + '/source_file')
    remove_if_exists(path + '/test-persist')

    with open(path + '/slng-test.conf', "w") as config_file:
        config_file.write(conf_destination.replace('SOURCE', 'file("%s");' % (path + '/source_file')).replace('DESTINATION', 'zmq();'))
    
    with open(path + '/source_file', 'w') as source_file:
        for i in xrange(0, NUMBER_OF_MESSAGES):
            source_file.write("Test message!!!\n")

    p = subprocess.Popen([os.environ['SYSLOG_NG'], '-Fdve', '-f', path + '/slng-test.conf', '-R', path + '/test-persist'], close_fds=True)
    while True:
        if res == NUMBER_OF_MESSAGES - 1:
            print("All messages arrived\n")
            break
        time.sleep(1)
    p.send_signal(subprocess.signal.SIGINT)

if __name__ == "__main__":
    main()
