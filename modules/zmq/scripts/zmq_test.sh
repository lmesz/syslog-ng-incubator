#!/bin/bash

rm -f /tmp/zmq

gcc pusher.c -o push -lzmq
cp ./syslog-ng.conf ~/install/syslog-ng/etc/syslog-ng.conf
~/install/syslog-ng/sbin/syslog-ng
./push 44444 1 &
sleep 5
num_of_lines=`grep -c Polip zmq`

if [ $num_of_lines == "1" ]
then
    kill -9 `ps aux | grep sbin/syslog-ng | grep -v grep | awk {'print $2'}`
    exit 0
fi
kill -9 `ps aux | grep sbin/syslog-ng | grep -v grep | awk {'print $2'}`
exit 1
