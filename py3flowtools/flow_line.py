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
PROTOCOL_NUMBERS = {
    'ICMP': 0x01,
    'TCP': 0x06,
    'UDP': 0x11,
    'L2TP': 0x73,
    'SCTP': 0x84,
}


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
    """
    Converts a line of CSV data from flow-tools' flow-tools or
    flowd's flowd-reader to a Python object whose fields match the flow-tools
    FlowSet object.
    """

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
    """
    Converts a line of CSV data from nfdump's output to a Python object whose
    fields match the flow-tools FlowSet object.
    """

    def __init__(self, line):
        line = [x.strip() for x in line.decode('ascii').split(',')]
        self.first = datetime.datetime.strptime(line[0], ISO8601_FMT)
        self.last = datetime.datetime.strptime(line[1], ISO8601_FMT)
        self.srcaddr = line[2]
        self.srcaddr_raw = inet_aton(line[2])
        self.dstaddr = line[4]
        self.dstaddr_raw = inet_aton(line[4])
        self.srcport = int(line[3])
        self.dstport = int(line[5])
        self.prot = PROTOCOL_NUMBERS.get(line[6], 0xFF)
        self.dPkts = int(line[7]) + int(line[8])
        self.dOctets = int(line[9]) + int(line[10])

        tcp_flags = ''.join('0' if b == '.' else '1' for b in line[11])
        self.tcp_flags = int(tcp_flags, 2)
