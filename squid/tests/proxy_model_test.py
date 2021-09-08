#!/bin/python3

import unittest
import tests.context
import re
from proxy_model import SearchBy
from proxy_model import ProxyModel
from proxy_model import Sentinel
from sys import argv


class TestProxyModel(unittest.TestCase):

    def setUp(self):
        self.pm = ProxyModel(database_path='tests/dbs')

    def test_empty(self):
        self.assertEqual(len(self.pm.registered_parser), 2,
                         "Esperado 2 parsers")
        self.assertEqual(self.pm.minimal_characters(), 0,
                         "Esperado 0 caracteres para avaliar")
        self.assertEqual(self.pm.maximal_characters(), 0,
                         "Esperado 0 caracteres para avaliar")

    def test_filename_location_in_linux(self):
        self.assertEqual(self.pm.filename_location(),
                         "tests/dbs/*.txt",
                         "Esperado path relativo com mascara")

    def test_split_buffer_empty(self):
        line_b = b""
        self.assertEqual(self.pm.split_buffer(line_b), [],
                         "Esperado conjunto vazio")
        pass

    def test_split_buffer_heading(self):
        line_b = b"no_proxy\r\n"
        self.assertEqual(self.pm.split_buffer(line_b), [b"no_proxy"],
                         "Esperado conjunto com um unico valor")

    def test_split_buffer_three_values(self):
        line_b = b"rename\twww.bol.com\t   www-bol-com\r\n"
        self.assertEqual(self.pm.split_buffer(line_b),
                         [b"rename", b"www.bol.com", b"www-bol-com"],
                         "Esperado conjunto de valores sem espacos")

    def test_clean(self):
        line_b = b"www.example.com"
        self.assertEqual(self.pm.clean(line_b),
                         b"www\.example\.com",
                         "Esperado valor de retorno como ER")

    def test_no_proxy(self):
        self.pm.traverse()
        cln = b'www\.example\.com'
        cln_er = re.compile(cln, re.IGNORECASE)
        pair_never = (cln, cln_er)
        self.assertIn(pair_never, self.pm.db_no_proxy_iter,
                      "Esperado par cadastrado")

    def test_proxy_domain_rename(self):
        self.pm.traverse()
        cln_er = re.compile(b'bol\.com\.br$', re.IGNORECASE)
        out = b'bol'
        out_er = re.compile(out + b'$', re.IGNORECASE)
        cln_out = b'bol.com.br'
        pair_going = (out, cln_er)
        pair_coming = (cln_out, out_er)
        self.assertIn(pair_going, self.pm.db_proxy_going_iter)
        self.assertIn(pair_coming, self.pm.db_proxy_coming_iter)

    def test_keysize(self):
        self.pm.traverse()
        # Menor valor de chave eh o menor valor tanto no no_proxy
        # quanto no proxy_domain_rename considerando tanto o lado
        # que vai quanto o lado que volta. Alem disso, pode ter
        # um caracter q mais. Aqui 4 por causa da re-escrita do
        # valor bol.com.br para bol que seria 3, mas tem no codigo
        # o dolar adicionado ($) ficando bol$.
        self.assertEqual(self.pm.minimal_characters(), 4,
                         "Esperado menor valor de chave")
        # length do periodicos + quantidade de barras pros pontos
        self.assertEqual(self.pm.maximal_characters(), 28,
                         "Esperado maior valor de chave")

    def test_search_no_proxy(self):
        self.pm.traverse()
        ret = self.pm.search(Sentinel(b'www.example.com'),
                             SearchBy.BY_NEVER_PROXY)
        expected_pair_never = (b'www\.example\.com',
                               re.compile(b'www\.example\.com', re.IGNORECASE))
        self.assertEqual(next(ret), expected_pair_never)

    def test_search_no_proxy_fail(self):
        self.pm.traverse()
        ret = self.pm.search(Sentinel(b'pudim.com.br'),
                             SearchBy.BY_NEVER_PROXY)
        with self.assertRaises(StopIteration):
            next(ret)

    def test_search_going(self):
        self.pm.traverse()
        ret = self.pm.search(Sentinel(b'bol.com.br'), SearchBy.BY_GOING)
        expected_pair_going = (b'bol',
                               re.compile(b'bol\.com\.br$', re.IGNORECASE))
        self.assertEqual(next(ret), expected_pair_going)

    def test_searchgoing_fail(self):
        self.pm.traverse()
        ret = self.pm.search(Sentinel(b'pudim.com.br'), SearchBy.BY_GOING)
        with self.assertRaises(StopIteration):
            next(ret)

    def test_search_coming(self):
        self.pm.traverse()
        ret = self.pm.search(Sentinel(b'bol'), SearchBy.BY_COMING)
        expected_pair_coming = (b'bol.com.br',
                                re.compile(b'bol$', re.IGNORECASE))
        self.assertEqual(next(ret), expected_pair_coming)

    def test_search_coming_fail(self):
        self.pm.traverse()
        ret = self.pm.search(Sentinel(b'pudim.com.br'), SearchBy.BY_COMING)
        with self.assertRaises(StopIteration):
            next(ret)

    def test_search_with_port(self):
        test_cases = [
            # a re-escrita remove a porta...
            (b'sos.com.br:1234', SearchBy.BY_GOING,
             (b's0S', re.compile(b'sos\\.com\\.br:1234', re.IGNORECASE))),
            # ...e a recoloca na volta
            (b's0S', SearchBy.BY_COMING,
             (b'sos.com.br:1234', re.compile(b's0S$', re.IGNORECASE))),
            # a re-escrita aqui mantera a porta!!!
            # porque a mesma nao se encontra cadastrada!
            (b'periodicos.capes.gov.br:1234', SearchBy.BY_GOING,
             (b'periodicos',
              re.compile(b'PeriOdicos\\.cAPes\\.gOv\\.br', re.IGNORECASE))),
        ]
        self.pm.traverse()
        for (p, s, e) in test_cases:
            ret = next(self.pm.search(Sentinel(p), s))
            self.assertEqual(ret, e)

        # esse outro exemplo eh do bol que possui na url
        # o cifrao dizendo que a mesma termina em .com.br
        with self.assertRaises(StopIteration):
            ret = next(self.pm.search(Sentinel(b'bol.com.br:443'),
                                      SearchBy.BY_GOING))

    def test_search_case_ok(self):
        test_cases = [
            (b'cdn-periodicos', SearchBy.BY_COMING,
             (b'.PeriOdicos.cAPes.gOv.br',
              re.compile(b'-periodicos$', re.IGNORECASE))),
            (b'periodicos', SearchBy.BY_COMING,
             (b'PeriOdicos.cAPes.gOv.br',
              re.compile(b'periodicos$', re.IGNORECASE))),
            (b'PeriOdicos.cAPes.gOv.br', SearchBy.BY_GOING,
             (b'periodicos',
              re.compile(b'PeriOdicos\\.cAPes\\.gOv\\.br', re.IGNORECASE))),
            (b'cdn.PeriOdicos.cAPes.gOv.br', SearchBy.BY_GOING,
             (b'-periodicos',
              re.compile(b'\\.PeriOdicos\\.cAPes\\.gOv\\.br', re.IGNORECASE))),
        ]
        self.pm.traverse()
        for (p, s, e) in test_cases:
            ret = next(self.pm.search(Sentinel(p), s))
            self.assertEqual(ret, e)

    def test_search_case_fail(self):
        test_cases = [
            (b'CDN-PERiODicos', SearchBy.BY_COMING,
             (b'.PeriOdicos.cAPes.gOv.br',
              re.compile(b'-periodicos$', re.IGNORECASE))),
            (b'pERioDicos', SearchBy.BY_COMING,
             (b'PeriOdicos.cAPes.gOv.br',
              re.compile(b'periodicos$', re.IGNORECASE))),
            (b'pERIoDicOS.CapES.GoV.BR', SearchBy.BY_GOING,
             (b'periodicos',
              re.compile(b'PeriOdicos\\.cAPes\\.gOv\\.br', re.IGNORECASE))),
            (b'cDN.pERIoDICOS.cAPes.gOV.Br', SearchBy.BY_GOING,
             (b'-periodicos',
              re.compile(b'\\.PeriOdicos\\.cAPes\\.gOv\\.br', re.IGNORECASE))),
        ]
        self.pm.traverse()
        for (p, s, e) in test_cases:
            ret = next(self.pm.search(Sentinel(p), s))
            self.assertEqual(ret, e)

    @unittest.skipUnless(argv[1]=="tests.proxy_model_test.TestProxyModel.test_check_database",
                        "manual run needed")
    def test_check_database(self):
        del self.pm
        self.pm = ProxyModel(
            database_path='src/helpers/dbs')
        self.pm.traverse()
        self.pm.check_database()
        print('minchars', self.pm.minimal_characters())
        print('maxchars', self.pm.maximal_characters())


class TestProxyModelNoProxy(unittest.TestCase):

    def setUp(self):
        class Mock(object):

            def __init__(self):
                self.db_no_proxy = dict()
                self.history_arg = None
                self.minimal_avail = 5
                self.maximal_avail = 5

            def clean(self, d):
                self.history_arg = d
                return b'\.example\.com'

        self.pm = Mock()

    def test_visit_no_proxy_sem_ajuste(self):
        line_b = b'.example.com'
        ProxyModel.visit_no_proxy(self.pm, line_b)
        self.assertEqual(self.pm.history_arg, line_b,
                         "Esperado no clean valor ja conhecido")
        expected = b'\.example\.com'
        self.assertEqual(list(self.pm.db_no_proxy.items())[0],
                         (expected,
                         (expected, re.compile(expected, re.IGNORECASE))),
                        "Esperado chave com ER")
        self.assertEqual(self.pm.minimal_avail, 5,
                        "Esperado valor minimo de caracters sem ajuste")
        self.assertEqual(self.pm.maximal_avail, 14,
                        "Esperado valor maximo de caracters sem ajuste")

    def test_visit_no_proxy_com_ajuste(self):

        self.pm.minimal_avail = 25
        self.pm.maximal_avail = 25
        line_b = b'.example.com'
        ProxyModel.visit_no_proxy(self.pm, line_b)
        self.assertEqual(self.pm.history_arg, line_b,
                         "Esperado no clean valor ja conhecido")
        expected = b'\.example\.com'
        self.assertEqual(list(self.pm.db_no_proxy.items())[0],
                         (expected, (expected, re.compile(expected, re.IGNORECASE))),
                        "Esperado chave com ER")
        self.assertEqual(self.pm.minimal_avail, 14,
                        "Esperado valor minimo de caracters sem ajuste")
        self.assertEqual(self.pm.maximal_avail, 25,
                        "Esperado valor maximo de caracters sem ajuste")


class TestProxyModelRenameDomain(unittest.TestCase):

    def setUp(self):

        class Mock(object):

            def __init__(self):
                self.db_proxy_going = dict()
                self.db_proxy_coming = dict()
                self.history_arg = None
                self.minimal_avail = 10
                self.maximal_avail = 10

            def clean(self, d):
                self.history_arg = d
                return b'\.example\.com'

        self.pm = Mock()

    def test_visit_rename_domain_minimal_character_changed(self):
        line_b1 = b'.example.com'
        line_b2 = b'example'
        ProxyModel.visit_rename_domain(self.pm, line_b1, line_b2)
        smallest = min(len(line_b1), len(line_b2)) + 1
        self.assertEqual(self.pm.minimal_avail, smallest,
                         "Esperado o menor das duas entradas ser o novo valor")
        self.assertEqual(self.pm.maximal_avail, 14,
                        "Esperado valor maximo de caracters sem ajuste")

    def test_visit_rename_domain_minimal_character(self):
        line_b1 = b'.example.com'
        line_b2 = b'-example-com'
        self.assertEqual(self.pm.minimal_avail, 10,
                         "Garantindo o valor inicial em 10")
        ProxyModel.visit_rename_domain(self.pm, line_b1, line_b2)
        self.assertEqual(self.pm.minimal_avail, 10,
                         "Esperado que o valor seja mantido no inicial")
        self.assertEqual(self.pm.maximal_avail, 14,
                        "Esperado valor maximo de caracters sem ajuste")

    def test_visit_rename_domain_going(self):
        line_b1 = b'.example.com'
        line_b1_er = re.compile(b'\.example\.com', re.IGNORECASE)
        line_b2 = b'-example-com'
        ProxyModel.visit_rename_domain(self.pm, line_b1, line_b2)
        dpg = list(self.pm.db_proxy_going.items())
        self.assertEqual(dpg[0],
                         (b'\.example\.com', (line_b2, line_b1_er)),
                         "Esperado do teste de transformacao de ida")

    def test_visit_rename_domain_coming(self):
        line_b1 = b'^example.com$'
        line_b1_er = re.compile(b'\.example\.com', re.IGNORECASE)
        line_b2 = b'-eXamPle-com'
        line_b2_er = re.compile(line_b2 + b'$', re.IGNORECASE)
        ProxyModel.visit_rename_domain(self.pm, line_b1, line_b2)
        dpc = list(self.pm.db_proxy_coming.items())
        self.assertEqual(dpc[0],
                         (line_b2 + b'$', (b'example.com', line_b2_er)),
                         "Esperado do teste de transformacao de volta")
