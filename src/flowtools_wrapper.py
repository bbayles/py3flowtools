# flowtools_wrapper.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import datetime
import io
import os
import socket
import struct
import sys

if sys.version_info.major < 3:
    import subprocess32 as subprocess
else:
    import subprocess

FLOW_EXPORT_ARGS = [
    'flow-export',
    '-f', '2',
]

ipv4_struct = struct.Struct('!I')
inet_aton = lambda x: ipv4_struct.unpack(socket.inet_aton(x))[0]
utc_time = lambda x: datetime.datetime.utcfromtimestamp(int(x) / 1000)


class FlowToolsLine(object):
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
        self.tcp_flags =int(line[19])


def FlowSet(file_path):
    with io.open(file_path, mode='rb') as flow_fd, \
         io.open(os.devnull, mode='wb') as DEVNULL:
        with subprocess.Popen(
                FLOW_EXPORT_ARGS,
                stdin=flow_fd,
                stdout=subprocess.PIPE,
                stderr=DEVNULL
            ) as proc:
            iterator = iter(proc.stdout.readline, b'')
            try:
                __ = next(iterator)
            except StopIteration:
                msg = 'Could not extract data from {}'.format(file_path)
                raise IOError(msg)
            for i, line in enumerate(iterator):
                parsed_line = FlowToolsLine(line)
                yield parsed_line
