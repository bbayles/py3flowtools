# flow_line.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license
from __future__ import division, print_function, unicode_literals

import datetime
import socket
import struct

inet_aton = lambda x: ipv4_struct.unpack(socket.inet_aton(x))[0]
ipv4_struct = struct.Struct('!I')
utc_time = lambda x: datetime.datetime.utcfromtimestamp(int(x) / 1000)


class FlowLine(object):
    def __init__(self, line):
        line = line.decode('ascii').split(',')
        self.first = utc_time(int(line[6]))
        self.last = utc_time(int(line[7]))
        self.srcaddr = line[10]
        self.srcaddr_raw = inet_aton(line[10])
        self.dstaddr = line[11]
        self.dstaddr_raw = inet_aton(line[11])
        self.srcport = int(line[15])
        self.dstport = int(line[16])
        self.prot = int(line[17])
        self.dOctets = int(line[5])
        self.dPkts = int(line[4])
        self.tcp_flags = int(line[19])
