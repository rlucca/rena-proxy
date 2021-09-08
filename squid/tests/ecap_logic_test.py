#!/bin/python3

import unittest
import tests.context
import ecap_logic_request as lrequest
import ecap_logic_reply as lreply
from proxy_controller import ProxyController
from time import time
from sys import argv
from random import randint


def profile_stat(func):
    def func_wrapper(*args, **kwargs):
        import cProfile, pstats, io
        pr = cProfile.Profile()
        pr.enable()
        func(*args, **kwargs)
        pr.disable()
        s = io.StringIO()
        ps = pstats.Stats(pr, stream=s).sort_stats('cumulative')
        ps.dump_stats('stat_{}.txt'.format(func.__name__))
    return func_wrapper


class TestEcapLogicRequest(unittest.TestCase):

    def setUp_header(self):
        self.sufixo = b'.ezN.periodicos.capes.gov.br'
        pc = ProxyController(self.sufixo,
                             database_path='tests/dbs')
        self.state = dict()
        self.state['pc'] = pc

    def test_begin(self):
        self.assertIsNone(lrequest.adapt_begin(None))

    def test_header(self):
        test_cases = [
            (b'Content-Length', b'1024', (False, None)),
            (b'Host', b'www.example.com', (False, None)),
            (b'Referer', b'www.example.com', (False, None)),
            (b'Referer', b'periodicos.ezN.periodicos.capes.gov.br',
             (True, b'PeriOdicos.cAPes.gOv.br')),
            (b'Referer',
             b'cdn-be12-periodicos.ezN.periodicos.capes.gov.br',
             (True, b'cdn-be12.PeriOdicos.cAPes.gOv.br')),
            (b'Referer',
             b's0S.ezN.periodicos.capes.gov.br',
             (True, b'sos.com.br:1234')),
        ]
        self.setUp_header()
        for (header, value, expected) in test_cases:
            ret = lrequest.adapt_header(self.state,
                                        header, value)
            self.assertEqual(ret, expected)

    def test_body(self):
        self.assertIsNone(lrequest.adapt_body(None, None))

    def test_end(self):
        self.assertIsNone(lrequest.adapt_end(None))


class TestEcapLogicReply(unittest.TestCase):

    def setUp(self):
        self.sufixo = b'.ezN.periodicos.capes.gov.br'
        self.pc = ProxyController(self.sufixo,
                                  database_path='tests/dbs')
        self.state = dict()
        self.state['pc'] = self.pc

    def test_begin(self):
        lreply.adapt_begin(self.state)
        self.assertNotEqual(len(self.state.keys()), 1)

    def test_content_type(self):
        test_cases = [
            (b"Content-Type", b"text/html; charset=utf-8", True),
            (b"Content-Type", b"text; charset=utf-8", False),
            (b"Content-Type", b"text/xml; charset=UTF-8", False),
        ]
        fixed_ret = (False, None)
        for (name, value, expected) in test_cases:
            self.state['html'] = False
            ret = lreply.adapt_header(self.state, name, value)
            self.assertEqual(ret, fixed_ret)
            self.assertEqual(self.state['html'], expected)

    def test_header(self):
        test_cases = [
            (b"Server", b"AtyponWS/7.1", (False, None)),
            (b"Cache-Control", b"private", (False, None)),
            (b"X-XSS-Protection", b"1; mode=block", (False, None)),
            (b"X-Content-Type-Options", b"nosniff", (False, None)),
            (b"Strict-Transport-Security", b"max-age=16070400", (False, None)),
            (b"X-Frame-Options", b"SAMEORIGIN", (False, None)),
            (b"P3P", b"CP=\"NOI DSP ADM OUR IND OTC\"", (False, None)),
            (b"Location", b"https://bol.com.br/?cookieSet=1",
             (True, b"https://bol.ezN.periodicos.capes.gov.br/?cookieSet=1")),
            (b"Set-Cookie", b"I2KBRCK=1; domain=.liebertpub.com; path=/; secure; expires=Thu, 09-Jul-2020 18:13:02 GMT",
             (True, b"I2KBRCK=1; domain=ezN.periodicos.capes.gov.br;path=/; secure; expires=Thu, 09-Jul-2020 18:13:02 GMT")),
            (b"Content-Length", b"73", (True, None)),
            (b"Cache-Control", b"private", (False, None)),
            (b"X-XSS-Protection", b"1; mode=block", (False, None)),
            (b"X-Content-Type-Options", b"nosniff", (False, None)),
            (b"Strict-Transport-Security", b"max-age=16070400", (False, None)),
            (b"X-Frame-Options", b"SAMEORIGIN", (False, None)),
            (b"Location", b"https://www.terra.com.br/",
             (True, b"https://terra.ezN.periodicos.capes.gov.br/")),
            (b"Content-Length", b"61", (True, None)),
            (b"Server", b"AtyponWS/7.1", (False, None)),
            (b"X-XSS-Protection", b"1; mode=block", (False, None)),
            (b"X-Content-Type-Options", b"nosniff", (False, None)),
            (b"Strict-Transport-Security", b"max-age=16070400", (False, None)),
            (b"X-Frame-Options", b"SAMEORIGIN", (False, None)),
            (b"Cache-Control", b"no-cache", (False, None)),
            (b"Pragma", b"no-cache", (False, None)),
            (b"X-Webstats-RespID", b"d913a7abb00b75381b8958c5074caf74", (False, None)),
            (b"Set-Cookie", b"SERVER=WZ6myaEXBLEUkiVfXKAZxw==; domain=.liebertpub.com; path=/; secure",
             (True, b"SERVER=WZ6myaEXBLEUkiVfXKAZxw==; domain=ezN.periodicos.capes.gov.br;path=/; secure")),
            (b"Set-Cookie", b"MAID=+UmureG7rQ9lENHcxBGyPQ==; domain=.liebertpub.com; path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT",
             (True, b"MAID=+UmureG7rQ9lENHcxBGyPQ==; domain=ezN.periodicos.capes.gov.br;path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT")),
            (b"Set-Cookie", b"MACHINE_LAST_SEEN=2019-07-10T11%3A13%3A03.030-07%3A00; domain=.liebertpub.com; path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT",
             (True, b"MACHINE_LAST_SEEN=2019-07-10T11%3A13%3A03.030-07%3A00; domain=ezN.periodicos.capes.gov.br;path=/; secure; expires=Tue, 05-May-2020 18:13:03 GMT")),
            (b"Set-Cookie", b"JSESSIONID=aaaZtTm03pp3J4DfcAzUw; domain=.liebertpub.com; path=/; secure; HttpOnly",
             (True, b"JSESSIONID=aaaZtTm03pp3J4DfcAzUw; domain=ezN.periodicos.capes.gov.br;path=/; secure; HttpOnly")),
            (b"Set-Cookie", b"JSESSIONID=aaaZtTm03pp3J4DfcAzUw; path=/; secure; HttpOnly", (False, None)),
            (b"Transfer-Encoding", b"chunked", (False, None)),
            (b"Date", b"Wed, 10 Jul 2019 18:13:03 GMT", (False, None)),
            (b"Vary", b"Accept-Encoding", (False, None)),
            (b"Location", b"https://sos.com.br:1234?",
             (True, b"https://s0S.ezN.periodicos.capes.gov.br?")),
            # Deveriamos aceitar? Considero que não...
            (b"Location", b"https://bol.com.br:443/",
             (False, None)),
        ]
        for (name, value, expected) in test_cases:
            ret = lreply.adapt_header(self.state, name, value)
            self.assertEqual(ret, expected, "header {}".format(name))

    def test_end(self):
        lreply.adapt_begin(self.state)
        self.assertIsNone(lreply.adapt_end(self.state))

        lreply.adapt_begin(self.state)
        self.state['context_atrb'] = b'>'
        self.state['context_text'] = b'</html>'
        self.assertEqual(lreply.adapt_end(self.state),
                         b'></html>')

    def test_body(self):
        test_cases = [
            (b"<html><body><a href=\"http://www.terra.com.br/videos/aulas/663/estado_de_mal_asmatico_no_pronto_socorro_pediatrico.htm\">estado de mal asmatico no pronto socorro pediatrico</a></body></html>",
            b"<html><body><a href=\"http://terra.ezN.periodicos.capes.gov.br/videos/aulas/663/estado_de_mal_asmatico_no_pronto_socorro_pediatrico.htm\">estado de mal asmatico no pronto socorro pediatrico</a></body></html>"),
            (b"<html><body><img src=\"http://www.terra.com.br/img/logo_medicinanet.gif\"></body></html>",
            b"<html><body><img src=\"http://terra.ezN.periodicos.capes.gov.br/img/logo_medicinanet.gif\"></body></html>"),
            (b"<a href=\"http://www.periodicos.capes.gov.br\">http://www.periodicos.capes.gov.br http://sos.com.br:1234</a>",
             b"<a href=\"http://www-periodicos.ezN.periodicos.capes.gov.br\">http://www-periodicos.ezN.periodicos.capes.gov.br http://s0S.ezN.periodicos.capes.gov.br</a>")
        ]
        for (raw, expected) in test_cases:
            state = { 'pc': self.pc }
            lreply.adapt_begin(state)
            state['html'] = True
            ret = lreply.adapt_body(state, raw, True)
            self.assertEqual(ret, expected)

    def test_falha_conhecida1_text(self):
        buf = b'''<body onload='javascript: setWindowName();setCookie("813B1RN2X2S7SQEP5LEPHXFCPRP3U2IK3F7HLP7FLMTKT3BXPE","buscador.periodicos.capes.gov.br");resizeIfrm();' alt="http://www.terra.com.br" style="background:none;background-color:#FFFFFF;background-image:none; url: 'buscador.periodicos.capes.gov.br/images/news/news.xml'" >'''
        expected = b'''<body onload='javascript: setWindowName();setCookie("813B1RN2X2S7SQEP5LEPHXFCPRP3U2IK3F7HLP7FLMTKT3BXPE","buscador-periodicos.ezN.periodicos.capes.gov.br");resizeIfrm();' alt="http://terra.ezN.periodicos.capes.gov.br" style="background:none;background-color:#FFFFFF;background-image:none; url: 'buscador-periodicos.ezN.periodicos.capes.gov.br/images/news/news.xml'" >'''

        lreply.adapt_begin(self.state)
        ret = lreply.adapt_body(self.state, buf, True)
        self.assertEqual(ret, expected)
        self.assertIsNone(lreply.adapt_end(self.state))

    def test_falha_conhecida1_html(self):
        buf = b'''<body onload='javascript: setWindowName();setCookie("813B1RN2X2S7SQEP5LEPHXFCPRP3U2IK3F7HLP7FLMTKT3BXPE","buscador.periodicos.capes.gov.br");resizeIfrm();' alt="http://www.terra.com.br" style="background:none;background-color:#FFFFFF;background-image:none; url: 'buscador.periodicos.capes.gov.br/images/news/news.xml'" >'''
        expected = b'''<body onload='javascript: setWindowName();setCookie("813B1RN2X2S7SQEP5LEPHXFCPRP3U2IK3F7HLP7FLMTKT3BXPE","buscador-periodicos.ezN.periodicos.capes.gov.br");resizeIfrm();' alt="http://terra.ezN.periodicos.capes.gov.br" style="background:none;background-color:#FFFFFF;background-image:none; url: 'buscador-periodicos.ezN.periodicos.capes.gov.br/images/news/news.xml'" >'''

        lreply.adapt_begin(self.state)
        self.state['html'] = True
        ret = lreply.adapt_body(self.state, buf, True)
        self.assertEqual(ret, expected)
        self.assertIsNone(lreply.adapt_end(self.state))

    def test_falha_conhecida2(self):
        buf = b'''<img src="http://www.example.com" alt="bUsCador.pERiodicos.cApes.goV.br/images/news/news.xml" title="http://www.example.com" />'''
        expected = b'''<img src="http://www.example.com" alt="bUsCador-periodicos.ezN.periodicos.capes.gov.br/images/news/news.xml" title="http://www.example.com" />'''

        lreply.adapt_begin(self.state)
        self.state['html'] = True
        ret = lreply.adapt_body(self.state, buf, True)
        self.assertEqual(ret, expected)
        self.assertIsNone(lreply.adapt_end(self.state))

    def test_falha_conhecida3(self):
        buf = b'//www.terra.com.br/'
        expected = b'http://terra.ezN.periodicos.capes.gov.br/'

        flag, changed = lreply.adapt_header(self.state, b'Location',
                                            b'http:' + buf)
        self.assertTrue(flag)
        self.assertEqual(changed, expected)

        flag, changed = lreply.adapt_header(self.state, b'Location', buf)
        self.assertTrue(flag)
        self.assertEqual(changed, expected)


class TestEcapLogicReplyFromFile(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.sufixo = b'.ezN.periodicos.capes.gov.br'
        cls.pc = ProxyController(cls.sufixo,
                                 database_path='src/helpers/dbs')

    def setUp(self):
        self.state = { 'pc': self.pc }

    @unittest.skipUnless("tests.ecap_logic_test.TestEcapLogicReplyFromFile" in argv[1],
                        "manual run needed")
    def test_cycle_communication_1(self):
        name_header = b"Content-Type"
        value_header = b"text/html; charset=utf-8"
        with open('tests/files/busca-periodicos_1.html', 'rb') as fd:
            buffer_read = fd.read(-1)
        with open('tests/files/busca-periodicos_1_result.html', 'rb') as fd:
            expected_read = fd.read(-1)
        # open channel...
        self.assertIsNone(lreply.adapt_begin(self.state))
        # send headers...
        flag, changed = lreply.adapt_header(self.state, name_header, value_header)
        self.assertFalse(flag)
        self.assertIsNone(changed)
        # send body...
        new_buffer = lreply.adapt_body(self.state, buffer_read)
        self.assertEqual(new_buffer, expected_read[:len(new_buffer)])
        # send remain body and close channel
        remain_buffer = lreply.adapt_end(self.state)
        self.assertIsNone(remain_buffer)

    @unittest.skipUnless("tests.ecap_logic_test.TestEcapLogicReplyFromFile" in argv[1],
                        "manual run needed")
    def test_cycle_communication_2(self):
        name_header = b"Content-Type"
        value_header = b"text/html; charset=utf-8"
        with open('tests/files/busca-periodicos_2.html', 'rb') as fd:
            buffer_read = fd.read(-1)
        with open('tests/files/busca-periodicos_2_result.html', 'rb') as fd:
            expected_read = fd.read(-1)
        # open channel...
        self.assertIsNone(lreply.adapt_begin(self.state))
        # send headers...
        flag, changed = lreply.adapt_header(self.state, name_header, value_header)
        self.assertFalse(flag)
        self.assertIsNone(changed)
        # send body...
        new_buffer = lreply.adapt_body(self.state, buffer_read)
        self.assertEqual(new_buffer, expected_read[:len(new_buffer)])
        # send remain body and close channel
        remain_buffer = lreply.adapt_end(self.state)
        self.assertIsNone(remain_buffer)

    @unittest.skipUnless("tests.ecap_logic_test.TestEcapLogicReplyFromFile" in argv[1],
                        "manual run needed")
    def test_cycle_communication_3(self):
        name_header = b"Content-Type"
        value_header = b"text/html; charset=utf-8"
        # open channel...
        self.assertIsNone(lreply.adapt_begin(self.state))
        # send headers...
        flag, changed = lreply.adapt_header(self.state, name_header, value_header)
        self.assertFalse(flag)
        self.assertIsNone(changed)
        # send remain body and close channel
        remain_buffer = lreply.adapt_end(self.state)
        self.assertIsNone(remain_buffer)

    @unittest.skipUnless("tests.ecap_logic_test.TestEcapLogicReplyFromFile" in argv[1],
                        "manual run needed")
    def test_cycle_communication_fragmented(self):
        with open('tests/files/jwatch_result.html', 'rb') as fd:
            expected_read = fd.read(-1).strip()
        # open channel...
        self.assertIsNone(lreply.adapt_begin(self.state))
        # send body...
        with open('tests/files/jwatch.html', 'rb') as fd:
            buffer_read = fd.read(80)
            while buffer_read:
                new_buffer = lreply.adapt_body(self.state, buffer_read)
                if new_buffer:
                    lnb = len(new_buffer)
                    self.assertEqual(new_buffer, expected_read[:lnb])
                    expected_read = expected_read[lnb:]
                buffer_read = fd.read(80)
        remain_buffer = lreply.adapt_end(self.state)
        if expected_read:
            self.assertEqual(remain_buffer, expected_read)
        else:
            self.assertIsNone(remain_buffer)

    @unittest.skipUnless("tests.ecap_logic_test.TestEcapLogicReplyFromFile" in argv[1],
                        "manual run needed")
    def test_html_cycle_communication_fragmented(self):
        name_header = b"Content-Type"
        value_header = b"text/html; charset=utf-8"
        with open('tests/files/busca-periodicos_2_result.html', 'rb') as fd:
            expected_read = fd.read(-1).strip()
        # open channel...
        self.assertIsNone(lreply.adapt_begin(self.state))
        # send headers...
        flag, changed = lreply.adapt_header(self.state, name_header, value_header)
        self.assertFalse(flag)
        self.assertIsNone(changed)
        # send body...
        with open('tests/files/busca-periodicos_2.html', 'rb') as fd:
            buffer_read = fd.read(randint(80, 8192))
            while buffer_read:
                new_buffer = lreply.adapt_body(self.state, buffer_read)
                if new_buffer:
                    lnb = len(new_buffer)
                    self.assertEqual(new_buffer, expected_read[:lnb])
                    expected_read = expected_read[lnb:]
                buffer_read = fd.read(randint(80, 8192))
        # send remain body and close channel
        remain_buffer = lreply.adapt_end(self.state)
        if expected_read:
            self.assertEqual(remain_buffer, expected_read)
        else:
            self.assertIsNone(remain_buffer)

