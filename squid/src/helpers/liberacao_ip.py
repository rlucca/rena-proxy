#!/bin/python3

from sys import stdin, stdout
# expiracao esta dentro do set
from cache_access import cache_check

label = ['SRC', 'INTERFACE']

# 1. SRC ja esta cadastrado? Libera o SRC;
while True:
    try:
        # vamos receber linhas como (sem as aspas):
        #     '10.0.2.2 10.0.2.15'
        line = stdin.readline()
        if not line:
            break
        data = dict(zip(label, line.strip().split(' ')))
        is_redirect = data['SRC'] == data['INTERFACE']
        if is_redirect or cache_check(data['SRC'], data['INTERFACE']):
            stdout.write("OK\n")
            stdout.flush()
            continue
    except EOFError:
        break
    except:
        # raise
        stdout.write("ERR log=\"unknown error\"\n")
        stdout.flush()
        break
    # line retains the new line
    stdout.write("ERR log=\"user need call /login\"\n")
    stdout.flush()
