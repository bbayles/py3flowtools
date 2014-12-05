# flow_line.py
# Copyright 2014 Bo Bayles (bbayles@gmail.com)
# See http://github.com/bbayles/py3flowtools for documentation and license
from __future__ import division, print_function, unicode_literals

import datetime
import itertools
import os
import unittest

import py3flowtools


class TestPy3FlowTools(unittest.TestCase):
    def setUp(self):
        file_path = os.path.join(
            os.path.dirname(__file__),
            'flowtools.log'
        )
        parser = py3flowtools.FlowSet(file_path)
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
        parser = py3flowtools.FlowSet(__file__)
        with self.assertRaises(IOError):
            list(parser)

    def test_attributes(self):
        attributes = [
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
