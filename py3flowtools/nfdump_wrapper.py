# nfdump_wrapper.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import io
import os
import sys

from .base_flow_wrapper import BaseFlowLog
from .flow_line import NfdumpLine

if sys.version_info.major < 3:
    import subprocess32 as subprocess
else:
    import subprocess

NFDUMP_ARGS = [
    'nfdump',
    '-r', None,
    '-q',
    '-o', 'fmt:%ts,%te,%sa,%sp,%da,%dp,%pr,%ipkt,%opkt,%ibyt,%obyt,%flg',
]


class NfdumpLog(BaseFlowLog):
    """
    Uses nfdump to parse a NetFlow log.
    """
    def _reader(self):
        args = NFDUMP_ARGS[:]
        args[2] = self._file_path

        with io.open(os.devnull, mode='wb') as DEVNULL:
            popen_kwargs = {
                'args': args, 'stdout': subprocess.PIPE, 'stderr': DEVNULL
            }
            with subprocess.Popen(**popen_kwargs) as proc:
                iterator = iter(proc.stdout.readline, b'')
                try:
                    next(iterator)
                except StopIteration:
                    msg = 'Could not extract data from {}'.format(file_path)
                    raise IOError(msg)
                for line in iterator:
                    parsed_line = NfdumpLine(line)
                    yield parsed_line
