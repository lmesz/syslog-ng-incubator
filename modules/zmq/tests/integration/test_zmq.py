#!/usr/bin/env python

import os, sys, subprocess, time, zmq

if not os.environ.has_key('SYSLOG_NG'):
    print("Please set where syslog-ng is else test will not work!")
    sys.exit(1)

def send_one_message():
    context = zmq.Context()
    zmq_socket = context.socket(zmq.PUSH)
    zmq_socket.bind("tcp://127.0.0.1:5558")
    for i in xrange(1):
        zmq_socket.send("Test message!\n")
    context.destroy()

def test_zmq_source():
    location = os.path.dirname(os.path.abspath(__file__))
    result_file_name = os.getcwd() + "/test_result"
    if os.path.isfile(location + "/syslog-source-output"):
        os.unlink(location + "/syslog-source-output")
    if os.path.isfile(result_file_name):
        os.unlink(result_file_name)
    p = subprocess.Popen([os.environ['SYSLOG_NG'], '-Fdve', '-f', location + '/syslog-ng-source.conf'])
    time.sleep(5)
    send_one_message()

    if not os.path.isfile(result_file_name):
        p.kill()
        print("#####################ERROR##################: Message didn't arrived, result file doesn't exists at " + result_file_name + " !")
        sys.exit(1)

    with open(result_file_name, "r") as res:
        if 'Test message' not in res.readline():
            print("#####################ERROR##################: Message didn't arrived, the result file doesn't contains the sent message!")
            p.kill()
            sys.exit(1)
    p.kill()
    print("ZMQ SOURCE: OK")

def test_zmq_destination():
    pass

def main():
    test_zmq_source()
    test_zmq_destination()

if __name__ == "__main__":
    main()
