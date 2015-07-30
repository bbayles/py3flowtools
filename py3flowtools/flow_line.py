# flow_line.py
# Copyright 2015 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import datetime
import socket
import struct

inet_aton = lambda x: ipv4_struct.unpack(socket.inet_aton(x))[0]
ipv4_struct = struct.Struct('!I')

_1E6 = pow(10, 6)
_1E9 = pow(10, 9)

ISO8601_FMT = '%Y-%m-%d %H:%M:%S.%f'


def get_utc_time(unix_secs, unix_nsecs, sysuptime, x):
    sysuptime_sec, sysuptime_msec = divmod(sysuptime, 1000)
    x_sec, x_msec = divmod(x, 1000)

    base_secs = unix_secs - sysuptime_sec + x_sec
    base_nsecs = unix_nsecs - (sysuptime_msec * _1E6) + (x_msec * _1E6)
    whole_secs, remainder_nsecs = divmod(base_nsecs, _1E9)

    ret = datetime.datetime.utcfromtimestamp(base_secs + whole_secs)
    ret = ret.replace(microsecond=remainder_nsecs // 1000)
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


class NfdumpLine(object):
    def __init__(self, line):
        # TODO 'fmt:%ts,%te,%sa,%sp,%da,%dp,%pr,%ipkt,%opkt,%ibyt,%obyt,%flg'
        line = line.decode('ascii').split(',')
        self.first = datetime.strptime(line[0], ISO8601_FMT)
        self.last = datetime.strptime(line[0], ISO8601_FMT)
        self.srcaddr = line[2]
        self.srcaddr_raw = inet_aton(line[2])
        self.dstaddr = line[3]
        self.dstaddr_raw = inet_aton(line[3])
        self.srcport = int(line[4])
        self.dstport = int(line[5])
        self.prot = line[6]  # Turn to integer?
        self.dPkts = int(line[7]) + int(line[8])
        self.dPkts = int(line[9]) + int(line[10])
        self.tcp_flags = int(line[11])
