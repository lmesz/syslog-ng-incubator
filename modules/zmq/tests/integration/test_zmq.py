#!/usr/bin/env python

import os
import sys
import subprocess
import time
import zmq
import socket

import threading

res = ""

if not os.environ.has_key('SYSLOG_NG'):
    print("Please set syslog-ng location else test will not work!")
    sys.exit(1)


def test_zmq_source():
    location = os.path.dirname(os.path.abspath(__file__))
    result_file_name = os.getcwd() + "/test_result_source"

    if os.path.isfile(result_file_name):
        os.unlink(result_file_name)

    p = subprocess.Popen([os.environ['SYSLOG_NG'], '-Fdve', '-f', location + '/syslog-ng-source.conf'])
    time.sleep(2)
    send_one_zmq_message()

    if not os.path.isfile(result_file_name):
        p.kill()
        print("#####################ERROR##################: Message didn't arrived, result file doesn't exists at " + result_file_name + " !")
        return False

    with open(result_file_name, "r") as res:
        if 'Test message' not in res.readline():
            print("#####################ERROR##################: Message didn't arrived, the result file doesn't contains the sent message!")
            p.kill()
            return False

    p.kill()
    return True

def send_one_zmq_message():
    context = zmq.Context()
    zmq_socket = context.socket(zmq.PUSH)
    zmq_socket.bind("tcp://127.0.0.1:5558")
    for i in xrange(1):
        zmq_socket.send("Test message!\n")
    context.destroy()

def test_zmq_destination():
    receiver_thread = threading.Thread(target=receive_zmq_message)
    receiver_thread.start()

    location = os.path.dirname(os.path.abspath(__file__))

    p = subprocess.Popen([os.environ['SYSLOG_NG'], '-Fdve', '-f', location + '/syslog-ng-destination.conf'])
    time.sleep(2)

    send_one_tcp_message()
    time.sleep(2)

    if "Test_message" in res:
        p.kill()
        return True
    p.kill()
    return False

def send_one_tcp_message():
    local_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    local_socket.connect(("127.0.0.1", 61000))
    local_socket.send("Test_message!\n")
    local_socket.close()
 
def receive_zmq_message():
    global res
    ctx = zmq.Context()
    sock = ctx.socket(zmq.PULL)
    sock.connect("tcp://0.0.0.0:5556")
    res += sock.recv()
    ctx.destroy()

def main():
    source_res = test_zmq_source()
    dest_res = test_zmq_destination()

    if source_res and dest_res:
        print("\n\n\n\n\n###########################################")
        print("ZMQ SOURCE AND DESTINATION: OK")
        print("###########################################")
    elif source_res:
        print("\n\n\n\n\n###########################################")
        print("ZMQ SOURCE : OK")
        print("ZMQ DESTINATION : ERROR")
        print("###########################################")
    elif dest_res:
        print("\n\n\n\n\n###########################################")
        print("ZMQ SOURCE : ERROR")
        print("ZMQ DESTINATION : OK")
        print("###########################################")

if __name__ == "__main__":
    main()
