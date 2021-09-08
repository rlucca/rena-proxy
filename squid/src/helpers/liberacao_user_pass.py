#!/bin/python3

from sys import stdin
from sys import stdout
from sys import argv
from urllib.parse import parse_qs

def read_dict(filename):
    from re import compile as c
    split = c(r'\s+')
    d = dict()
    with open(filename, 'r') as fd:
        for line in fd:
            user, pwd = split.split(line.strip())
            d[user] = pwd
    return d


def check_pass(d, data):
    # /login?user=xxx&pass=yyy, 7 referente a posicao '/login?'
    query = parse_qs(data['PATH'][7:], keep_blank_values=True)
    try:
        # porque zero? Sempre eh uma lista e checamos o primeiro...
        u = query['user'][0]
        p = query['pass'][0]
        return d[u] == p
    except KeyError:
        pass
    return False


def process_line(label, passwd_list):
    try:
        # vamos receber linhas como (sem as aspas):
        #     '10.0.2.2 10.0.2.15 /login?user=teste&pass=exemplo&url='
        line = stdin.readline()
        if not line:
            return 1
        data = dict(zip(label, line.strip().split(' ')))
        if '/login' in data['PATH'] and check_pass(passwd_list, data):
            stdout.write("OK\n")
            stdout.flush()
            return 0
    except EOFError:
        return 1
    except:
        # raise
        stdout.write("ERR\n")
        stdout.flush()
        return 1
    stdout.write("ERR\n")
    stdout.flush()
    return 0


if __name__ == '__main__':
    if len(argv) != 2:
        print("Informe apenas um arquivo de senhas")
        exit(3)

    label = ['SRC', 'INTERFACE', 'PATH']
    # passwd_list = read_dict(argv[1])
    passwd_list = {'teste': 'exemplo'}

    # 1. SRC ja esta cadastrado? Libera o SRC;
    # 2. PATH Inicia com /login ? Cadastra ele e libera o SRC;
    while True:
        result = process_line(label, passwd_list)
        if result > 0:
            break
        else:
            continue
