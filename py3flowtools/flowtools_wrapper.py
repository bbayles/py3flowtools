# flowtools_wrapper.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import io
import os
import sys

from .flow_line import FlowLine

if sys.version_info.major < 3:
    import subprocess32 as subprocess
else:
    import subprocess

FLOW_EXPORT_ARGS = [
    'flow-export',
    '-f', '2',
]


class FlowToolsLog(object):
    def __init__(self, file_path):
        self._file_path = file_path

    def __iter__(self):
        self._parser = self._reader()
        return self

    def __next__(self):
        return next(self._parser)

    def next(self):
        """
        next method included for compatibility with Python 2
        """
        return self.__next__()

    def _reader(self):
        with io.open(self._file_path, mode='rb') as flow_fd, \
                io.open(os.devnull, mode='wb') as DEVNULL:
            with subprocess.Popen(
                    FLOW_EXPORT_ARGS,
                    stdin=flow_fd,
                    stdout=subprocess.PIPE,
                    stderr=DEVNULL
            ) as proc:
                iterator = iter(proc.stdout.readline, b'')
                try:
                    next(iterator)
                except StopIteration:
                    msg = 'Could not extract data from {}'.format(
                        self._file_path
                    )
                    raise IOError(msg)
                for line in iterator:
                    parsed_line = FlowLine(line)
                    yield parsed_line
