#!/bin/bash

cleanup()
{
    rm -f /tmp/zmq
}

compile_pusher()
{
    export PKG_CONFIG_PATH=$HOME/install/libzmq/lib/pkgconfig
    gcc pusher.c -o push -lzmq
}

find_syslog_ng_pid()
{
    echo `ps aux | grep sbin/syslog-ng | grep -v grep | awk {'print $2'}`
}

kill_syslog_ng()
{
    kill -9 $(find_syslog_ng_pid)
}

reload_syslog_ng()
{
    kill -1 $(find_syslog_ng_pid)
}

check_if_zmq_source_can_receive_one_message()
{
    echo "Starting testcase: check_if_zmq_source_can_receive_one_message"
    cleanup
    compile_pusher

    cp ./syslog-ng.conf_port44444 ~/install/syslog-ng/etc/syslog-ng.conf
    ~/install/syslog-ng/sbin/syslog-ng

    ./push 44444 1 &
    sleep 3
    num_of_lines=`grep -c Polip /tmp/zmq`

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
    compile_pusher

    cp ./syslog-ng.conf_port44444 ~/install/syslog-ng/etc/syslog-ng.conf
    ~/install/syslog-ng/sbin/syslog-ng
    ./push 44444 1 &

    cp ./syslog-ng.conf_port44445 ~/install/syslog-ng/etc/syslog-ng.conf

    reload_syslog_ng
    ./push 44445 1 &

    sleep 3
    num_of_lines=`grep -c Polip /tmp/zmq`

    if [ $num_of_lines == "2" ]
    then
            kill_syslog_ng 9
            return 0
    fi
    kill_syslog_ng 9
    echo "Failed testcase: check_if_zmq_source_can_receive_one_message"
    exit 1

}

check_if_zmq_source_can_receive_one_message
check_if_zmq_source_receive_messages_on_another_port_that_changed_during_reload
echo "#######All testcases passed!#######"
