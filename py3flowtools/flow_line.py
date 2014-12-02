# flow_line.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license
from __future__ import division, print_function, unicode_literals

import datetime
import socket
import struct

inet_aton = lambda x: ipv4_struct.unpack(socket.inet_aton(x))[0]
ipv4_struct = struct.Struct('!I')


def get_utc_time(unix_secs, unix_nsecs, sysuptime, x):
    # unix_secs is the whole number of seconds since the epoch
    # unix_nsecs is the number of residual nanoseconds
    unix_time = unix_secs + (unix_nsecs / 1E9)

    # sysuptime is the number of milliseconds the collecting device has been on
    unix_base = unix_time - (sysuptime / 1000)

    # x (either first or start) is the number of milliseconds of uptime at the
    # start or end of the flow
    ret = datetime.datetime.utcfromtimestamp(unix_base + (x / 1000))

    return ret


class FlowLine(object):
    def __init__(self, line):
        line = line.decode('ascii').split(',')

        # Base times
        unix_secs = int(line[0])
        unix_nsecs = int(line[1])
        sysuptime = int(line[2])
        first = int(line[6])
        last = int(line[7])

        # Fields to expose
        self.dOctets = int(line[5])
        self.dPkts = int(line[4])
        self.first = get_utc_time(unix_secs, unix_nsecs, sysuptime, first)
        self.last = get_utc_time(unix_secs, unix_nsecs, sysuptime, last)
        self.srcaddr = line[10]
        self.srcaddr_raw = inet_aton(line[10])
        self.dstaddr = line[11]
        self.dstaddr_raw = inet_aton(line[11])
        self.srcport = int(line[15])
        self.dstport = int(line[16])
        self.prot = int(line[17])
        self.tcp_flags = int(line[19])
