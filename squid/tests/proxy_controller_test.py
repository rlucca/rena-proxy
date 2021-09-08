#!/bin/python3

import unittest
import tests.context
from proxy_controller import ProxyController

class TestProxyController(unittest.TestCase):

    def setUp(self):
        self.sufixo = b'.ezN.periodicos.capes.gov.br'
        self.pc = ProxyController(self.sufixo,
                                  database_path='tests/dbs')

    def test_no_op(self):
        self.assertIsNotNone(self.pc.model)
        self.assertIsNotNone(self.pc.transformMatch)
        self.assertIsNotNone(self.pc.suffixMatch)
        self.assertEqual(self.pc.get_suffix(),
                         self.sufixo[1:])

    def test_extract_url_from_path(self):
       base_link = b'://ezN.periodicos.capes.gov.br/login?url='
       test_cases = [
            (b'http', b'www.example.com', b'http://www.example.com'),
            (b'http', b'://www.example.com', b'http://www.example.com'),
            (b'http', b'http://www.example.com', b'http://www.example.com'),
            (b'https', b'www.example.com', b'https://www.example.com'),
            (b'https', b'http://www.example.com', b'http://www.example.com'),
       ]
       for (schema, url, expected) in test_cases:
            ret = self.pc.extract_url_from_path(base_link + url, schema)
            self.assertEqual(ret, expected)

    def test_split_url(self):
        test_cases = [
            (b'http://www.example.com',
             b'http:', b'www.example.com', b''),
            (b'hTTp://www.example.com?',
             b'http:', b'www.example.com', b'?'),
            (b'hTTP://www.example.com/',
             b'http:', b'www.example.com', b'/'),
            (b'www.example.com',
             b'', b'www.example.com', b''),
            (b'www.example.com?Q=AbAcATe',
             b'', b'www.example.com', b'?Q=AbAcATe'),
            (b'www.example.com/query?q=42',
             b'', b'www.example.com', b'/query?q=42'),
            (b'user1:pwd@www.example.com',
             b'', b'www.example.com', b''),
            (b'HTTPS://USER1:PWD@WWW.EXAMPLE.COM:8080?zITOs',
             b'https:', b'WWW.EXAMPLE.COM:8080', b'?zITOs'),
            (b'www.example.com#anyThing',
             b'', b'www.example.com', b'#anyThing'),
            (b'//www.example.com#anyThing',
             b'', b'www.example.com', b'#anyThing'),
            (b'/moo#anyThing',
             b'', b'', b'/moo#anyThing'),
            (b'/moo?anyThing',
             b'', b'', b'/moo?anyThing'),
            (b'text/css',
             b'', b'text', b'/css'),
        ]
        for (link, expected1, expected2, expected3) in test_cases:
            ret_schema, ret_domain, ret_query_or_path = \
                self.pc.split_url(link)
            self.assertEqual(ret_schema, expected1)
            self.assertEqual(ret_domain, expected2)
            self.assertEqual(ret_query_or_path, expected3)

    def test_find_any_of(self):
        test_cases = [
            (b'213', 3),
            (b'abacate', 7),
            (b'abacate#topic1', 7),
            (b'i/dir', 1),
            (b'i#dir', 1),
            (b'123i?dir', 4),
        ]
        for (target, expected) in test_cases:
            ret = self.pc._find_any_of(b'/?#', target)
            self.assertEqual(ret, expected)

    def test_undo_redirect(self):
        test_cases = [
            (b'http://www.example.com/img/index.cHm?RestPar=42avcE!Rop',
             b'www.example.com',
             None),
            (b'http://terra.ezN.periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop',
             b'terra.ezN.periodicos.capes.gov.br',
             b'http://www.terra.com.br/img/index.cHm?RestPar=42avcE!Rop'),
            (b'https://bol.ezN.periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop',
             b'bol.ezN.periodicos.capes.gov.br',
             b'https://bol.com.br/img/index.cHm?RestPar=42avcE!Rop'),
            (b'https://cdn-periodicos.ezN.periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop',
             b'cdn-periodicos.ezN.periodicos.capes.gov.br',
             b'https://cdn.PeriOdicos.cAPes.gOv.br/img/index.cHm?RestPar=42avcE!Rop'),
            (b'https://periodicos.ezN.periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop',
             b'periodicos.ezN.periodicos.capes.gov.br',
             b'https://PeriOdicos.cAPes.gOv.br/img/index.cHm?RestPar=42avcE!Rop'),
            (b'https://link-periodicos.ezN.periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop',
             b'link-periodicos.ezN.periodicos.capes.gov.br',
             b'https://link.PeriOdicos.cAPes.gOv.br/img/index.cHm?RestPar=42avcE!Rop'),
            (b'https://bol.ezN.periodicos.capes.gov.br:8080',
             b'bol.ezN.periodicos.capes.gov.br:8080',
             b'https://bol.com.br'),
            (b'https://bol.ezN.periodicos.capes.gov.br:8080',
             b'bol.ezN.periodicos.capes.gov.br',
             b'https://bol.com.br:8080'),
            (b'https://bol.ezN.periodicos.capes.gov.br?',
             b'bol.ezN.periodicos.capes.gov.br',
             b'https://bol.com.br?'),
            (b'https://bol.ezN.periodicos.capes.gov.br/',
             b'bol.ezN.periodicos.capes.gov.br',
             b'https://bol.com.br/'),
            (b'https://s0s.ezN.periodicos.capes.gov.br/',
             b's0s.ezN.periodicos.capes.gov.br',
             b'https://sos.com.br:1234/'),
            (b'https://bol.eZn.PEriODICOS.CApes.gov.br/',
             b'bol.eZn.PEriODICOS.CApes.gov.br',
             b'https://bol.com.br/'),
            (b'bol.eZn.PEriODICOS.CApes.gov.br/ziTOs?metaphor=42',
             b'bol.eZn.PEriODICOS.CApes.gov.br',
             b'bol.com.br/ziTOs?metaphor=42'),
            # a porta que antes nao existia e eh perdida como esta do lado
            # esquerdo no proxy_domain_rename, re aparece pra ir no servidor
            (b's0S.ezn.periodicos.capes.gov.br',
             b's0S.ezn.periodicos.capes.gov.br',
             b'sos.com.br:1234'),
        ]
        for (url, domain, expected) in test_cases:
            ret = self.pc.undo_redirect(url, domain)
            self.assertEqual(ret, expected)

    def test_do_redirect(self):
        test_cases = [
            (b'www.terra.com.br',
             None,
             (True, b'http://terra.ezN.periodicos.capes.gov.br')),
            (b'http://bol.com.br',
             b'ezN.periodicos.capes.gov.br',
             (True, b'http://bol.ezN.periodicos.capes.gov.br')),
            # aqui esse endereco nao encontra-se cadastrado com porta
            # e ainda eh direto que a url DEVE terminar em .com.br
            (b'http://bol.com.br:8080',
             b'ezN.periodicos.capes.gov.br',
             (False, b'http://bol.com.br:8080')),
            (b'http://bol.com.bra',
             b'ezN.periodicos.capes.gov.br',
             (False, b'http://bol.com.bra')),
            (b'http://www.pudim.com.br',
             b'ezN.periodicos.capes.gov.br',
             (False, b'http://www.pudim.com.br')),
            (b'http://periodicos.capes.gov.br',
             b'ezN.periodicos.capes.gov.br',
             (True, b'http://periodicos.ezN.periodicos.capes.gov.br')),
            (b'http://cdn-be123.periodicos.capes.gov.br',
             None,
             (True, b'http://cdn-be123-periodicos.ezN.periodicos.capes.gov.br')),
            (b'http://cdn-be123.periodicos.capes.gov.br/',
             None,
             (True, b'http://cdn-be123-periodicos.ezN.periodicos.capes.gov.br/')),
            (b'http://cdn-be123.periodicos.capes.gov.br?',
             None,
             (True, b'http://cdn-be123-periodicos.ezN.periodicos.capes.gov.br?')),
            # a porta nao existe no lado esquerdo do periodicos,
            # a mesma eh removida manualmente e nao voltara
            (b'http://periodicos.capes.gov.br:8080/?',
             None,
             (True, b'http://periodicos.ezN.periodicos.capes.gov.br/?')),
            (b'http://periodicos.capes.gov.br?q=42',
             None,
             (True, b'http://periodicos.ezN.periodicos.capes.gov.br?q=42')),
            (b'http://periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop',
             None,
             (True, b'http://periodicos.ezN.periodicos.capes.gov.br/img/index.cHm?RestPar=42avcE!Rop')),
            (b'http://periodicos.capes.gov.br?RestPar=42avcE!Rop',
             None,
             (True, b'http://periodicos.ezN.periodicos.capes.gov.br?RestPar=42avcE!Rop')),
            (b'http://periodicos.capes.gov.br#lambda',
             None,
             (True, b'http://periodicos.ezN.periodicos.capes.gov.br#lambda')),
            # a porta existente eh perdida, mas sera restaurada na volta...
            (b'http://sos.com.br:1234',
             None,
             (True, b'http://s0S.ezN.periodicos.capes.gov.br')),
            # Um dos sites fez essa porcaria...
            (b'//sos.com.br:1234',
             None,
             (True, b'http://s0S.ezN.periodicos.capes.gov.br')),
            (b'//www.rsc-cdn.org/www.terra.com.br/api-v2/content/stylesheets/headerfooter/RSCHeaderFooterNew.css?3.7.1.0',
             None,
             (False, b'//www.rsc-cdn.org/www.terra.com.br/api-v2/content/stylesheets/headerfooter/RSCHeaderFooterNew.css?3.7.1.0')),
        ]
        for (url, domain, expected) in test_cases:
            ret = self.pc.do_redirect(url, domain)
            self.assertEqual(ret, expected)

    def test_do_all_redirects(self):
        test_cases = [
            (b'www.example.com', b'www.example.com'),
            (b'https://periodicos.CAPEs.gov.br',
             b'https://periodicos.ezN.periodicos.capes.gov.br'),
            (b'https://www.periodicos.capes.gov.br',
             b'https://www-periodicos.ezN.periodicos.capes.gov.br'),
            (b'https://cdn23-www.periodicos.capes.gov.br',
             b'https://cdn23-www-periodicos.ezN.periodicos.capes.gov.br'),
            # a porta eh perdida aqui tambem para ser recolocada somente no acesso
            (b'https://sos.com.br:1234', b'https://s0S.ezN.periodicos.capes.gov.br'),
            # Um dos sites fez isso...
            (b'//sos.com.br:1234', b'//s0S.ezN.periodicos.capes.gov.br'),
            # Outro site fez isso e pela inconsistencia, nao conhecer o site com squid
            # muda o acesso para um popup "prove que eh humano"...
            (b'//www.rsc-cdn.org/www.terra.com.br/api-v2', b'//www.rsc-cdn.org/www.terra.com.br/api-v2'),
        ]
        for (url, expected) in test_cases:
            ret = self.pc.do_all_redirects(url)
            self.assertEqual(ret, expected)

