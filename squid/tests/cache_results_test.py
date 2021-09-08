#!/bin/python3

import unittest
import tests.context
from proxy_controller import cache_results, cached_results

class TestCacheResults(unittest.TestCase):

    def setUp(self):
        cached_results.clear()

    def test_cache_results(self):
        mock = lambda s, u, o: 1
        call = cache_results(mock)
        self.assertEqual(len(cached_results), 0, "len deve ser zero")
        self.assertEqual(call(b'', b'a', b'1'), 1, "esperando 1")
        self.assertEqual(len(cached_results), 1, "len deve ser 1")
        self.assertEqual(call(b'', b'a', b'2'), 1, "esperando 1")
        self.assertEqual(len(cached_results), 2, "len deve ser 2")
        self.assertEqual(call(b'', b'a', b'1'), 1, "esperando 1")
        self.assertEqual(len(cached_results), 2, "len deve ser 2")  # no new element

    def test_cache_results_max_len(self):
        mock = lambda s, u, o: 2
        call = cache_results(mock)
        # populate all 11 elements...
        for i in range(1, 12):
            cached_results[b'a!' + bytes([i])] = 1
        self.assertEqual(len(cached_results), 11, "len deve ser 11")
        # insert a new element
        call(b'', b'a', b'13')
        # ...
        self.assertEqual(len(cached_results), 11, "len deve ser 11")
        self.assertIn(2, cached_results.values(), "esperando novo valor exista")
