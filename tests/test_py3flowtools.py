# flow_line.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license
from __future__ import division, print_function, unicode_literals

import datetime
import itertools
import os
import unittest

import py3flowtools


FLOW_LINE_ATTRIBUTES = [
    'dOctets',
    'dPkts',
    'first',
    'last',
    'srcaddr',
    'srcaddr_raw',
    'dstaddr',
    'dstaddr_raw',
    'srcport',
    'dstport',
    'prot',
    'tcp_flags',
]


class TestFlowToolsWrapper(unittest.TestCase):
    def setUp(self):
        file_path = os.path.join(
            os.path.dirname(__file__),
            'flowtools.log'
        )
        parser = py3flowtools.FlowToolsLog(file_path)
        self.flows = list(parser)

    def test_length(self):
        # The test file should have 20 flows
        self.assertEqual(len(self.flows), 20)

    def test_first_calc(self):
        # Compare the first field to known values
        self.flows.sort(key=lambda x: x.first)
        known_values = (
            datetime.datetime(2014, 12, 5, 0, 26, 53, 82806),
            datetime.datetime(2014, 12, 5, 1, 59, 37, 226517),
            datetime.datetime(2014, 12, 5, 19, 28, 43, 999867)
        )
        for i, expected in zip([0, 10, -1], known_values):
            actual = self.flows[i].first
            self.assertEqual(actual, expected)

    def test_last_calc(self):
        # Compare the last field to known values
        self.flows.sort(key=lambda x: x.last)
        known_values = (
            datetime.datetime(2014, 12, 5, 0, 26, 53, 89806),
            datetime.datetime(2014, 12, 5, 1, 59, 37, 231517),
            datetime.datetime(2014, 12, 5, 19, 28, 44, 1867)
        )
        for i, expected in zip([0, 10, -1], known_values):
            actual = self.flows[i].last
            self.assertEqual(actual, expected)

    def test_bogus_file(self):
        parser = py3flowtools.FlowToolsLog('flowd.log')
        with self.assertRaises(IOError):
            list(parser)

    def test_attributes(self):
        attributes = FLOW_LINE_ATTRIBUTES
        for flow, attribute in itertools.product(self.flows, attributes):
            self.assertTrue(hasattr(flow, attribute))

    def test_ip_addresses(self):
        D = {
            self.flows[1].srcaddr: '192.0.2.121',
            self.flows[2].srcaddr_raw: 3221225999,
            self.flows[3].dstaddr: '192.0.2.222',
            self.flows[4].dstaddr_raw: 3221225995,
        }
        for k, v in D.items():
            self.assertEqual(k, v)

    def test_fields(self):
        D = {
            self.flows[1].dOctets: 13156,
            self.flows[2].dPkts: 16,
            self.flows[3].srcport: 954,
            self.flows[4].dstport: 42777,
            self.flows[5].prot: 6,
            self.flows[6].prot: 17,
        }
        for k, v in D.items():
            self.assertEqual(k, v)


class TestFlowdWrapper(unittest.TestCase):
    def setUp(self):
        file_path = os.path.join(
            os.path.dirname(__file__),
            'flowd.log'
        )
        parser = py3flowtools.FlowdLog(file_path)
        self.flows = list(parser)

    def test_length(self):
        # The test file should have 20 flows
        self.assertEqual(len(self.flows), 20)

    def test_first_calc(self):
        # Compare the first field to known values
        self.flows.sort(key=lambda x: x.first)
        known_values = (
            datetime.datetime(2014, 11, 26, 13, 34, 55, 928908),
            datetime.datetime(2014, 11, 26, 13, 34, 56, 48780),
            datetime.datetime(2014, 11, 26, 13, 34, 56, 150597),
        )
        for i, expected in zip([0, 10, -1], known_values):
            actual = self.flows[i].first
            self.assertEqual(actual, expected)

    def test_last_calc(self):
        # Compare the last field to known values
        self.flows.sort(key=lambda x: x.last)
        known_values = (
            datetime.datetime(2014, 11, 26, 13, 34, 56, 152053),
            datetime.datetime(2014, 11, 26, 13, 34, 56, 531199),
            datetime.datetime(2014, 11, 26, 13, 34, 56, 896285)
        )
        for i, expected in zip([0, 10, -1], known_values):
            actual = self.flows[i].last
            self.assertEqual(actual, expected)

    def test_bogus_file(self):
        parser = py3flowtools.FlowdLog('flowtools.log')
        with self.assertRaises(IOError):
            list(parser)

    def test_attributes(self):
        attributes = FLOW_LINE_ATTRIBUTES
        for flow, attribute in itertools.product(self.flows, attributes):
            self.assertTrue(hasattr(flow, attribute))

    def test_ip_addresses(self):
        D = {
            self.flows[1].srcaddr: '192.0.2.28',
            self.flows[2].srcaddr_raw: 3221226212,
            self.flows[3].dstaddr: '192.0.2.187',
            self.flows[4].dstaddr_raw: 3221226025,
        }
        for k, v in D.items():
            self.assertEqual(k, v)

    def test_fields(self):
        D = {
            self.flows[1].dOctets: 95095,
            self.flows[2].dPkts: 16,
            self.flows[3].srcport: 16335,
            self.flows[4].dstport: 14915,
            self.flows[5].prot: 17,
            self.flows[6].prot: 6,
        }
        for k, v in D.items():
            self.assertEqual(k, v)


class TestNetFlowLog(unittest.TestCase):
    def test_flowtools(self):
        file_path = os.path.join(
            os.path.dirname(__file__),
            'flowtools.log'
        )
        netflow_log = list(py3flowtools.NetFlowLog(file_path))
        self.assertEqual(len(netflow_log), 20)

    def test_flowd(self):
        file_path = os.path.join(
            os.path.dirname(__file__),
            'flowd.log'
        )
        netflow_log = list(py3flowtools.NetFlowLog(file_path))
        self.assertEqual(len(netflow_log), 20)

    def test_bogus(self):
        file_path = os.path.join(
            os.path.dirname(__file__),
            '__init__.py'
        )
        with self.assertRaises(IOError):
            list(py3flowtools.NetFlowLog(file_path))
