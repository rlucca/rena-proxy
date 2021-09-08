#!/bin/python3

import unittest
import tests.context
import liberacao_user_pass
import mock
from sys import stderr

class TestLiberacaoUserPass(unittest.TestCase):

    def setUp(self):
        self.label = ['SRC', 'INTERFACE', 'PATH']

    def call_processline(self, pl=None):
        return liberacao_user_pass.process_line(self.label, pl)

    @mock.patch('liberacao_user_pass.stdin.readline')
    def test_empty_line(self, mock_readline):
        mock_readline.return_value = ''
        self.assertEqual(self.call_processline(), 1)

    @mock.patch('liberacao_user_pass.stdout.flush')
    @mock.patch('liberacao_user_pass.stdout.write')
    @mock.patch('liberacao_user_pass.stdin.readline')
    def test_dirty_line(self, mock_readline, mock_write, mock_flush):
        mock_readline.return_value = ' \t  \n'
        self.assertEqual(self.call_processline(), 1)
        mock_write.assert_any_call('ERR log="unknown error"\n')
        mock_flush.assert_called()

    @mock.patch('liberacao_user_pass.stdin.readline')
    def test_eof(self, mock_readline):
        mock_readline.side_effect = EOFError('end of file')
        self.assertEqual(self.call_processline(), 1)

    @mock.patch('liberacao_user_pass.stdout.flush')
    @mock.patch('liberacao_user_pass.stdout.write')
    @mock.patch('liberacao_user_pass.stdin.readline')
    @mock.patch('liberacao_user_pass.cache_check')
    def test_cache_ok(self, ccheck, readl, write, flush):
        line = '10.0.2.43 10.0.2.0 /login?user=naoImporta&pass=a&url=\n'
        readl.return_value = line
        ccheck.return_value = True
        self.assertEqual(self.call_processline(), 0)
        write.assert_any_call('OK\n')
        flush.assert_called()

    @mock.patch('liberacao_user_pass.stdout.flush')
    @mock.patch('liberacao_user_pass.stdout.write')
    @mock.patch('liberacao_user_pass.stdin.readline')
    @mock.patch('liberacao_user_pass.cache_check')
    @mock.patch('liberacao_user_pass.check_pass')
    def test_user_pass_nok(self, cpass, ccheck, readl, write, flush):
        line = '10.0.2.43 10.0.2.0 /login?user=naoImporta&pass=a&url=\n'
        readl.return_value = line
        ccheck.return_value = False
        cpass.return_value = False
        self.assertEqual(self.call_processline(), 0)
        write.assert_any_call('ERR log="user need call /login"\n')
        flush.assert_called()

    @mock.patch('liberacao_user_pass.stdout.flush')
    @mock.patch('liberacao_user_pass.stdout.write')
    @mock.patch('liberacao_user_pass.stdin.readline')
    @mock.patch('liberacao_user_pass.cache_check')
    @mock.patch('liberacao_user_pass.cache_set')
    @mock.patch('liberacao_user_pass.check_pass')
    def test_user_pass_ok(self, cpass, cset, ccheck, readl, write, flush):
        line = '10.0.2.43 10.0.2.0 /login?user=naoImporta&pass=a&url=\n'
        readl.return_value = line
        ccheck.return_value = False
        cpass.return_value = True
        self.assertEqual(self.call_processline(), 0)
        cset.assert_called()
        write.assert_any_call('OK\n')
        flush.assert_called()

    @mock.patch('liberacao_user_pass.stdout.flush')
    @mock.patch('liberacao_user_pass.stdout.write')
    @mock.patch('liberacao_user_pass.stdin.readline')
    @mock.patch('liberacao_user_pass.cache_check')
    def test_user_pass_fail(self, ccheck, readl, write, flush):
        line = '10.0.2.43 10.0.2.0 /login?user=a1&pass=fail&url=\n'
        user_list = {'a1': 'b3'}
        readl.return_value = line
        ccheck.return_value = False
        self.assertEqual(self.call_processline(user_list), 0)
        write.assert_any_call('ERR log="user need call /login"\n')
        flush.assert_called()

    @mock.patch('liberacao_user_pass.stdout.flush')
    @mock.patch('liberacao_user_pass.stdout.write')
    @mock.patch('liberacao_user_pass.stdin.readline')
    @mock.patch('liberacao_user_pass.cache_check')
    @mock.patch('liberacao_user_pass.cache_set')
    def test_user_pass_ok(self, cset, ccheck, readl, write, flush):
        line = '10.0.2.43 10.0.2.0 /login?user=a1&pass=b3&url=\n'
        user_list = {'a1': 'b3'}
        readl.return_value = line
        ccheck.return_value = False
        self.assertEqual(self.call_processline(user_list), 0)
        cset.assert_called()
        write.assert_any_call('OK\n')
        flush.assert_called()
