# flowtools_wrapper.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import io
import os
import sys

from .base_flow_wrapper import BaseFlowLog
from .flow_line import FlowLine

if sys.version_info.major < 3:
    import subprocess32 as subprocess
else:
    import subprocess

ERR_MSG = 'Could not extract data from {}'

FLOWD_READER_ARGS = [
    'flowd-reader',
    '-v',  # Verbose output
    '-c',  # CSV formatting
    '-U',  # UTC timestamps
    '{file_path:}'
]


class FlowdLog(BaseFlowLog):
    """
    Uses flowd-reader to parse a log that flow-tools can read.
    """

    def read_flow(self):
        """
        read_flow method included for compatibility with flowd module
        """
        return self.__next__()

    def _reader(self):
        args = FLOWD_READER_ARGS[:]
        args[-1] = self._file_path
        with io.open(os.devnull, mode='wb') as DEVNULL:
            with subprocess.Popen(
                    args,
                    stdout=subprocess.PIPE,
                    stderr=DEVNULL
            ) as proc:
                iterator = iter(proc.stdout.readline, b'')
                try:
                    # Skip the headers
                    next(iterator)
                    next(iterator)
                except StopIteration:
                    raise IOError(ERR_MSG.format(self._file_path))
                line = None
                for line in iterator:
                    parsed_line = FlowLine(line)
                    yield parsed_line
                else:
                    if line is None:
                        raise IOError(ERR_MSG.format(self._file_path))
