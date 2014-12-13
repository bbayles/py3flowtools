# __init__.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from .flowd_wrapper import FlowdLog
from .flowtools_wrapper import FlowToolsLog

__all__ = ['NetFlowLog', 'FlowToolsLog', 'FlowdLog']


def NetFlowLog(file_path):
    """
    Tries to parse the given file with flow-tools. If that fails, it tries to
    use flowd.
    """
    # Attempt to use flow-tools
    try:
        for flow in FlowToolsLog(file_path):
            yield flow
    except IOError:
        pass
    else:
        return
    # Attempt to use flowd
    try:
        for flow in FlowdLog(file_path):
            yield flow
    except IOError as err_msg:
        raise IOError('Could not parse {}: {}'.format(file_path, err_msg))
