#!/bin/bash

rm -f /tmp/zmq

cp ./syslog-ng.conf ~/install/syslog-ng/etc/syslog-ng.conf
~/install/syslog-ng/sbin/syslog-ng
./push 44444 1 &
sleep 5
num_of_lines=`grep -c Polip /tmp/zmq`

if [ $num_of_lines == "1" ]
then
    exit 0
fi
exit 1
