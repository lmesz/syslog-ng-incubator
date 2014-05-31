#!/usr/bin/env python
import time
from zmq_client import ZMQClient

import control
import log


config = """@version: 3.5

source s_file { file("/tmp/pol.ip"); };

destination d_zmq { zmq(); };

log { source(s_tcp); destination(d_zmq); };

""" % locals()


def main():
    client = ZMQClient(port=5555)
    client.daemon = True
    client.start()
    control.start_syslogng(config, True)
    time.sleep(10)
    control.stop_syslogng()

if __name__ == "__main__":
    main()
