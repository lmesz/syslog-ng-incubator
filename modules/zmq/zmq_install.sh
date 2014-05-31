#!/bin/bash

wget https://github.com/zeromq/libzmq/archive/master.zip -O /tmp/master.zip
cd /tmp
unzip master.zip
cd libzmq-master && ./autogen.sh && ./configure --prefix=/usr && make && make install
