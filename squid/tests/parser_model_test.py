#!/bin/python3

import unittest
import tests.context
from proxy_model import BaseParser
from proxy_model import NoProxyParser
from proxy_model import RenameDomainParser

class TestBaseParser(unittest.TestCase):

    def setUp(self):
        self.bp = BaseParser()

    def test_string_empty(self):
        line_splited = [b'']
        with self.assertRaises(NotImplementedError):
            self.bp.accept(line_splited[0])

    def test_string_not_registered(self):
        # note o S no final do heading...
        line_splited = [b'proxy_domain_renames', b'www.example.com']
        with self.assertRaises(NotImplementedError):
            self.bp.accept(line_splited[0])

    def test_string_no_proxy(self):
        line_splited = [b'no_proxy', b'www.example.com']
        with self.assertRaises(NotImplementedError):
            self.bp.accept(line_splited[0])

    def test_string_rename(self):
        line_splited = [b'proxy_domain_rename', b'www.example.com', 'example']
        with self.assertRaises(NotImplementedError):
            self.bp.accept(line_splited[0])


class TestNoProxyParser(unittest.TestCase):

    def setUp(self):
        self.bp = NoProxyParser()

    def test_string_empty(self):
        line_splited = [b'']
        self.assertEqual(self.bp.accept(line_splited[0]), False, "Esperado valor falso")

    def test_string_not_registered(self):
        # note o S no final do heading...
        line_splited = [b'proxy_domain_renames', b'www.example.com']
        self.assertEqual(self.bp.accept(line_splited[0]), False, "Esperado valor falso")

    def test_string_no_proxy_invalid_missing_information(self):
        line_splited = [b'no_proxy']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")
        with self.assertRaises(NotImplementedError):
            self.bp.visit(None, line_splited)

    def test_string_no_proxy_invalid_too_much_information(self):
        line_splited = [b'no_proxy', b'www.example.com', b'example']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")
        with self.assertRaises(NotImplementedError):
            self.bp.visit(None, line_splited)

    def test_string_no_proxy_ok(self):
        line_splited = [b'no_proxy', b'www.example.com']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")


        class Mock(object):
            self.history_arg = None

            def visit_no_proxy(self, x):
                self.history_arg = x


        SN = Mock()
        with self.assertRaises(AttributeError):
            SN.history_arg
        self.bp.visit(SN, line_splited)
        self.assertEqual(SN.history_arg, line_splited[1],
                          "Esperado parametro igual ao segundo elemento do array")

    def test_string_rename(self):
        line_splited = [b'proxy_domain_rename', b'www.example.com', b'example']
        self.assertEqual(self.bp.accept(line_splited[0]), False, "Esperado valor falso")


class TestRenameDomainParser(unittest.TestCase):

    def setUp(self):
        self.bp = RenameDomainParser()

    def test_string_empty(self):
        line_splited = [b'']
        self.assertEqual(self.bp.accept(line_splited[0]), False, "Esperado valor falso")

    def test_string_not_registered(self):
        # note o S no final do heading...
        line_splited = [b'proxy_domain_renames', b'www.example.com']
        self.assertEqual(self.bp.accept(line_splited[0]), False, "Esperado valor falso")

    def test_string_no_proxy(self):
        line_splited = [b'no_proxy', b'www.example.com']
        self.assertEqual(self.bp.accept(line_splited[0]), False, "Esperado valor falso")

    def test_string_rename_missing_data_1(self):
        line_splited = [b'proxy_domain_rename']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")
        with self.assertRaises(NotImplementedError):
            self.bp.visit(None, line_splited)

    def test_string_rename_missing_data_2(self):
        line_splited = [b'proxy_domain_rename', b'www.example.com']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")
        with self.assertRaises(NotImplementedError):
            self.bp.visit(None, line_splited)

    def test_string_rename_too_much_data(self):
        line_splited = [b'proxy_domain_rename', b'www.example.com', b'example', b'something']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")
        with self.assertRaises(NotImplementedError):
            self.bp.visit(None, line_splited)

    def test_string_rename(self):
        line_splited = [b'proxy_domain_rename', b'www.example.com', b'example']
        self.assertEqual(self.bp.accept(line_splited[0]), True, "Esperado valor verdadeiro")


        class Mock(object):
            self.history_arg = None

            def visit_rename_domain(self, x, y):
                self.history_arg = [x, y]


        SN = Mock()
        with self.assertRaises(AttributeError):
            SN.history_arg
        self.bp.visit(SN, line_splited)
        self.assertEqual(len(SN.history_arg), 2,
                          "Esperado o recebimento de dois parametros")
        self.assertEqual(SN.history_arg, line_splited[1:],
                          "Esperado parametros recebidos iguais ao array")
