# __init__.py
# Copyright 2015 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from .flowd_wrapper import FlowdLog
from .flowtools_wrapper import FlowToolsLog
from .nfdump_wrapper import NfdumpLog

__all__ = ['NetFlowLog', 'FlowToolsLog', 'FlowdLog', 'NfdumpLog']


def NetFlowLog(file_path):
    """
    Tries to parse the given file with each of the known formats.
    """
    for cls in (NfdumpLog, FlowToolsLog, FlowdLog):
        try:
            for flow in cls(file_path):
                yield flow
        except IOError:
            pass
        else:
            return

    # Nothing worked!
    raise IOError('Could not parse {}'.format(file_path))
