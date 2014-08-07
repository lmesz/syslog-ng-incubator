#!/bin/bash

cleanup()
{
    sudo rm -f /tmp/zmq
}

compile_pusher()
{
    gcc pusher.c -o push -lzmq
}

find_syslog_ng_pid()
{
    echo `ps aux | grep sbin/syslog-ng | grep -v grep | awk {'print $2'}`
}

kill_syslog_ng()
{
    sudo kill -9 $(find_syslog_ng_pid)
}

reload_syslog_ng()
{
    sudo kill -1 $(find_syslog_ng_pid)
}

check_if_zmq_source_can_receive_one_message()
{
    echo "Starting testcase: check_if_zmq_source_can_receive_one_message"
    cleanup

    sudo cp ./syslog-ng.conf_port44444 /etc/syslog-ng/syslog-ng.conf
    sudo LD_LIBRARY_PATH=/usr/local/lib syslog-ng

    LD_LIBRARY_PATH=/usr/local/lib ./push 44444 1 &
    sleep 3
    num_of_lines=`sudo grep -c Polip /tmp/zmq`

    if [ $num_of_lines == "1" ]
    then
            kill_syslog_ng
            return 0
    fi
    kill_syslog_ng
    echo "Failed testcase: check_if_zmq_source_can_receive_one_message"
    exit 1
}

check_if_zmq_source_receive_messages_on_another_port_that_changed_during_reload()
{
    echo "Starting testcase: check_if_zmq_source_receive_messages_on_another_port_that_changed_during_reload"
    cleanup

    sudo cp ./syslog-ng.conf_port44444 /etc/syslog-ng/syslog-ng.conf
    sudo LD_LIBRARY_PATH=/usr/local/lib syslog-ng

    LD_LIBRARY_PATH=/usr/local/lib ./push 44444 1 &

    sudo cp ./syslog-ng.conf_port44445 /etc/syslog-ng/syslog-ng.conf

    reload_syslog_ng
    LD_LIBRARY_PATH=/usr/local/lib ./push 44445 1 &

    sleep 3
    num_of_lines=`sudo grep -c Polip /tmp/zmq`

    if [ $num_of_lines == "2" ]
    then
            kill_syslog_ng 9
            return 0
    fi
    kill_syslog_ng 9
    echo "Failed testcase: check_if_zmq_source_can_receive_one_message"
    exit 1

}

if [ "$1" == "with-pusher" ]
then
    echo "Called with parameter so pusher will be compiled too"
    compile_pusher
fi

check_if_zmq_source_can_receive_one_message
check_if_zmq_source_receive_messages_on_another_port_that_changed_during_reload
echo "#######All testcases passed!#######"
