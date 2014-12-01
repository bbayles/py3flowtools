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

FLOWD_READER_ARGS = [
    'flowd-reader',
    '-v',  # Verbose output
    '-c',  # CSV formatting
    '-U',  # UTC timestamps
    '{file_path:}'
]


def FlowLog(file_path):
    args = FLOWD_READER_ARGS[:]
    args[-1] = file_path
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
                msg = 'Could not extract data from {}'.format(file_path)
                raise IOError(msg)
            for line in iterator:
                parsed_line = FlowLine(line)
                yield parsed_line
