#!/bin/python3

import unittest
import tests.context
import url_rewriter
import mock
from proxy_controller import ProxyController
from sys import stderr
from sys import argv
from time import time
from random import shuffle

class TestUrlRewriter(unittest.TestCase):

    def setUp(self):
        self.sufixo = b'.ezN.periodicos.capes.gov.br'
        if argv[1] == 'tests.url_rewriter_test.TestUrlRewriter.test_slow':
            db_path = 'src/helpers/dbs'
        else:
            db_path = 'tests/dbs'
        self.pc = ProxyController(self.sufixo,
                                  database_path=db_path)

    @mock.patch('url_rewriter.stdin.readline')
    def test_empty_line(self, mock_readline):
        mock_readline.return_value = ''
        self.assertEqual(url_rewriter.process_line(self.pc, None), 1)

    @mock.patch('url_rewriter.stdin.readline')
    def test_dirty_line(self, mock_readline):
        mock_readline.return_value = '   \n'
        self.assertEqual(url_rewriter.process_line(self.pc, None), 0)

    @mock.patch('url_rewriter.stdin.readline')
    def test_eof(self, mock_readline):
        mock_readline.side_effect = EOFError('end of file')
        self.assertEqual(url_rewriter.process_line(self.pc, None), 1)

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_write_with_none(self, mock_flush, mock_write, mock_readline):
        def raise_error_on_none(x):
            if x == 'http://ezN.periodicos.capes.gov.brNone':
                raise TypeError('none passed to write')
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        mock_write.side_effect = raise_error_on_none
        url_rewriter.process_line(self.pc, 'None')
        mock_write.assert_any_call("ERR log=\"unknown error\"")
        mock_write.assert_any_call("\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_no_proxy(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://www.example.com 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("http://www.example.com")
        mock_write.assert_any_call("\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_no_proxy_without_database(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://www.example.net 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("http://www.example.net")
        mock_write.assert_any_call("\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_no_proxy_force(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=https://www.example.net 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK status=302 url=\"")
        mock_write.assert_any_call("https://www.example.net")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_terra(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=http://www.terra.com.br 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        # stderr.write(str(mock_write.mock_calls))
        mock_write.assert_any_call("OK status=302 url=\"")
        mock_write.assert_any_call("http://terra.ezN.periodicos.capes.gov.br")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_port_ok(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=http://sos.com.br:1234 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK status=302 url=\"")
        mock_write.assert_any_call("http://s0S.ezN.periodicos.capes.gov.br")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_port_nok(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=http://www.sos.com.br:1234 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK status=302 url=\"")
        # note que esta cadastrado no sistema: sos.com.br:1234 e nao ^sos.com.br:1234
        # no segundo caso, nao seria substituido. Cabe considerar tambem que o caso
        # abaixo resultara em uma falha de certificado se for https
        mock_write.assert_any_call("http://www.s0S.ezN.periodicos.capes.gov.br")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_sub_domain_ok(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=http://www.periodicos.capes.gov.br 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK status=302 url=\"")
        mock_write.assert_any_call("http://www-periodicos.ezN.periodicos.capes.gov.br")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_bol_ok(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=http://bol.com.br 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK status=302 url=\"")
        mock_write.assert_any_call("http://bol.ezN.periodicos.capes.gov.br")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_bol_nok(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://ezN.periodicos.capes.gov.br/login?url=http://bol.com.bri 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK status=302 url=\"")
        mock_write.assert_any_call("http://bol.com.bri")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()
        
    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_terra_reverse(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://terra.ezN.periodicos.capes.gov.br/AbC 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        # stderr.write(str(mock_write.mock_calls))
        mock_write.assert_any_call("OK rewrite-url=\"")
        mock_write.assert_any_call("http://www.terra.com.br/AbC")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_port_reverse(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://s0S.ezN.periodicos.capes.gov.br/ 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        #stderr.write(str(mock_write.mock_calls))
        mock_write.assert_any_call("OK rewrite-url=\"")
        mock_write.assert_any_call("http://sos.com.br:1234/")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_port_reverse_error(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://www.s0S.ezN.periodicos.capes.gov.br/ 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        #stderr.write(str(mock_write.mock_calls))
        mock_write.assert_any_call("OK rewrite-url=\"")
        mock_write.assert_any_call("http://www.sos.com.br:1234/")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_sub_domain_reverse(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://www-periodicos.ezN.periodicos.capes.gov.br 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        #stderr.write(str(mock_write.mock_calls))
        mock_write.assert_any_call("OK rewrite-url=\"")
        mock_write.assert_any_call("http://www.PeriOdicos.cAPes.gOv.br")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_rename_bol_reverse(self, mock_flush, mock_write, mock_readline):
        mock_readline.return_value = 'http://bol.ezN.periodicos.capes.gov.br/titleHistory 10.0.12.1/10.0.12.0 - GET myip=10.0.2.16 myport=444\n'
        self.assertEqual(url_rewriter.process_line(self.pc, 'asd'), 0)
        mock_write.assert_any_call("OK rewrite-url=\"")
        mock_write.assert_any_call("http://bol.com.br/titleHistory")
        mock_write.assert_any_call("\"\n")
        mock_flush.assert_called()

    @unittest.skipUnless(argv[1]=="tests.url_rewriter_test.TestUrlRewriter.test_slow",
                        "manual run needed")
    @mock.patch('url_rewriter.stdin.readline')
    @mock.patch('url_rewriter.stdout.write')
    @mock.patch('url_rewriter.stdout.flush')
    def test_slow(self, mock_flush, mock_write, mock_readline):
        test_cases=[
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br',
             'OK rewrite-url="https://www.liebertpub.com"\n'),
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br/pb/css/t-1-v0/default.css',
             'OK rewrite-url="https://www.liebertpub.com/pb/css/t-1-v0/default.css"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/img/bg_body.png',
             'OK rewrite-url="https://www.medicinanet.com.br/img/bg_body.png"\n'),
            ('http://www-oxfordmusiconline-com.ezN.periodicos.capes.gov.br?',
             'OK rewrite-url="http://www.oxfordmusiconline.com?"\n'),
            ('https://www-jstor-org.ezN.periodicos.capes.gov.br/px/client/main.min.js',
             'OK rewrite-url="https://www.jstor.org/px/client/main.min.js"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/editorial.asp?cmd=verulthome&idc=0',
             'OK rewrite-url="http://www.medicinanet.com.br/editorial.asp?cmd=verulthome&idc=0"\n'),
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br/na101/home/literatum/publisher/mal/journals/content/wound/2019/wound.2019.8.issue-1/wound.2019.8.issue-1/20190108/wound.2019.8.issue-1.cover.jpg',
             'OK rewrite-url="https://www.liebertpub.com/na101/home/literatum/publisher/mal/journals/content/wound/2019/wound.2019.8.issue-1/wound.2019.8.issue-1/20190108/wound.2019.8.issue-1.cover.jpg"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/bannerflutuante.aspx?cp=0',
             'OK rewrite-url="http://www.medicinanet.com.br/bannerflutuante.aspx?cp=0"\n'),
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br/resources/pb-ui/lib/font-awesome-4.6.3/fonts/fontawesome-webfont.woff2?v=4.6.3',
             'OK rewrite-url="https://www.liebertpub.com/resources/pb-ui/lib/font-awesome-4.6.3/fonts/fontawesome-webfont.woff2?v=4.6.3"\n'),
            ('http://www-ebscohost-com.ezN.periodicos.capes.gov.br/img/bg_btn_login.png',
             'OK rewrite-url="http://www.ebscohost.com/img/bg_btn_login.png"\n'),
            ('http://www-rsc-org.ezN.periodicos.capes.gov.br/categorias/acp-medicine.htm?mobile=off',
             'OK rewrite-url="http://www.rsc.org/categorias/acp-medicine.htm?mobile=off"\n'),
            ('https://ezN.periodicos.capes.gov.br/login?user=teste&pass=exemplo&url=http://www.oxfordmusiconline.com/',
             'OK status=302 url="http://www-oxfordmusiconline-com.ezN.periodicos.capes.gov.br/"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/css/estilo.css',
             'OK rewrite-url="http://www.medicinanet.com.br/css/estilo.css"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/img/ico_novo.gif',
             'OK rewrite-url="https://www.medicinanet.com.br/img/ico_novo.gif"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/css/estrutura.css',
             'OK rewrite-url="https://www.medicinanet.com.br/css/estrutura.css"\n'),
            ('https://www-jstor-org.ezN.periodicos.capes.gov.br/publishers',
             'OK rewrite-url="https://www.jstor.org/publishers"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/NajaNET/Naja_Publisher.Methods,v0.0.0.0.ashx',
             'OK rewrite-url="http://www.medicinanet.com.br/NajaNET/Naja_Publisher.Methods,v0.0.0.0.ashx"\n'),
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br/products/mal/releasedAssets/css/print.css',
             'OK rewrite-url="https://www.liebertpub.com/products/mal/releasedAssets/css/print.css"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/css/reset.css',
             'OK rewrite-url="http://www.medicinanet.com.br/css/reset.css"\n'),
            ('https://www-jstor-org.ezN.periodicos.capes.gov.br/publisher/amacom',
             'OK rewrite-url="https://www.jstor.org/publisher/amacom"\n'),
            ('http://www-rsc-org.ezN.periodicos.capes.gov.br/categorias/acp-medicine.htm?mobile=off',
             'OK rewrite-url="http://www.rsc.org/categorias/acp-medicine.htm?mobile=off"\n'),
            ('https://ezN.periodicos.capes.gov.br/login?user=teste&pass=exemplo&url=https://cdn-be231.scifinder.cas.org/asharg/titlePublisher/12355223/authorname/Michael',
             'OK status=302 url="https://cdn-be231-scifinder-cas-org.ezN.periodicos.capes.gov.br/asharg/titlePublisher/12355223/authorname/Michael"\n'),
            ('https://www-jstor-org.ezN.periodicos.capes.gov.br/px/xhr/api/v1/collector/pxPixel.gif?appId=PXu4K0s8nX',
             'OK rewrite-url="https://www.jstor.org/px/xhr/api/v1/collector/pxPixel.gif?appId=PXu4K0s8nX"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/NajaNET/Naja_System_Az,v0.0.0.0.ashx',
             'OK rewrite-url="https://www.medicinanet.com.br/NajaNET/Naja_System_Az,v0.0.0.0.ashx"\n'),
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br/favicon.ico',
             'OK rewrite-url="https://www.liebertpub.com/favicon.ico"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/videos/aulas/199/radiografia_de_torax.htm',
             'OK rewrite-url="http://www.medicinanet.com.br/videos/aulas/199/radiografia_de_torax.htm"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/',
             'OK rewrite-url="http://www.medicinanet.com.br/"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/img/bg_menu.png',
             'OK rewrite-url="http://www.medicinanet.com.br/img/bg_menu.png"\n'),
            ('https://www-jstor-org.ezN.periodicos.capes.gov.br/px/xhr/api/v1/collector',
             'OK rewrite-url="https://www.jstor.org/px/xhr/api/v1/collector"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/img/list_setinha.gif',
             'OK rewrite-url="http://www.medicinanet.com.br/img/list_setinha.gif"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/scripts.aspm?v=1.0.0.31229',
             'OK rewrite-url="http://www.medicinanet.com.br/scripts.aspm?v=1.0.0.31229"\n'),
            ('https://ezN.periodicos.capes.gov.br/login?user=teste&pass=exemplo&url=http://www.CabDirect.orG/',
             'OK status=302 url="http://www-cabdirect-org.ezN.periodicos.capes.gov.br/"\n'),
            ('https://static-content-springer-com.ezN.periodicos.capes.gov.br/cover/journal/343.jpg',
             'OK rewrite-url="https://static-content.springer.com/cover/journal/343.jpg"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/NajaNET/Naja_System,v0.0.0.0.ashx',
             'OK rewrite-url="https://www.medicinanet.com.br/NajaNET/Naja_System,v0.0.0.0.ashx"\n'),
            ('http://ezN.periodicos.capes.gov.br/login?user=teste&pass=exemplo&url=https://www.umi.com/',
             'OK status=302 url="https://www-umi.ezN.periodicos.capes.gov.br/"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/img/bloqueado.png',
             'OK rewrite-url="https://www.medicinanet.com.br/img/bloqueado.png"\n'),
            ('https://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/categorias/artigos.htm?mobile=off',
             'OK rewrite-url="https://www.medicinanet.com.br/categorias/artigos.htm?mobile=off"\n'),
            ('https://www-liebertpub-com.ezN.periodicos.capes.gov.br/loi/wound',
             'OK rewrite-url="https://www.liebertpub.com/loi/wound"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/NajaNET/Naja_System_i5,v0.0.0.0.ashx',
             'OK rewrite-url="http://www.medicinanet.com.br/NajaNET/Naja_System_i5,v0.0.0.0.ashx"\n'),
            ('http://www-medicinanet-com-br.ezN.periodicos.capes.gov.br/css/dropdown.css',
             'OK rewrite-url="http://www.medicinanet.com.br/css/dropdown.css"\n'),
            ('http://ezN.periodicos.capes.gov.br/login?user=teste&pass=exemplo&url=http://br.proquestk12.com/img/Bundle/232i',
             'OK status=302 url="http://br-proquestk12.ezN.periodicos.capes.gov.br/img/Bundle/232i"\n'),
        ]


        total_i = 0
        total = 0
        for I in range(1000):
            shuffle(test_cases)
            start_time = time()

            for (sent, expected) in test_cases:
                full_line = sent + ' 10.0.2.2/10.0.2.2 - GET myip=10.0.2.15 myport=8081'
                mock_write.reset_mock()
                mock_readline.return_value = full_line
                self.assertEqual(url_rewriter.process_line(self.pc, '/login?url=http://www.periodicos.capes.gov.br'), 0)
                # pegamos a resposta em uma so linha pra simplificar a comparacao...
                reply = ''.join(str(call.args[0]) for call in mock_write.mock_calls)
                #stderr.write(str(reply))
                self.assertEqual(expected, reply)

            end_time = time()
            delta_time = end_time - start_time
            total_i += 1.0
            total = (total + delta_time) / total_i

        # note que aceitavel seria uma media entre 0.0001 e 0.0005 sempre
        self.assertTrue(total <= 0.002)
        stderr.write('50 requests averaging {0:1.4f}s'.format(total))
