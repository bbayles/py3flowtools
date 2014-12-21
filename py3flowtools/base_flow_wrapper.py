# flow_wrapper.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license

from __future__ import division, print_function, unicode_literals

import abc


class BaseFlowLog(object):
    __metaclass__ = abc.ABCMeta

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

    @abc.abstractmethod
    def _reader(self):
        pass
