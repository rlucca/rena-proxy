#!/bin/python3

from queue import Queue, Full
from threading import Thread
from os import access, R_OK
from sys import stdin
from sys import stdout
from urllib.parse import parse_qs
from cache_access import CacheAccess


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


def work_thread():
    ca = CacheAccess(unixsocket, expiration)
    while True:
        item = q.get()
        if not item:
            break

        data = dict(zip(label, item.strip().split(' ')))
        flag = ca.cache_check(data['SRC'], data['INTERFACE'])

        if flag or ('/login' in data['PATH'] and check_pass(passwd_list, data)):
            stdout.write("{} OK\n".format(data.get('CHANNEL', 0)))
            stdout.flush()
            q.task_done()
            ca.cache_set(data['SRC'], data['INTERFACE'])
            continue

        stdout.write("{} ERR\n".format(data.get('CHANNEL', 0)))
        stdout.flush()
        q.task_done()


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--thread", type=int, default=4,
                        help="number of working threads")
    parser.add_argument("-w", "--pass", type=str,
                        default="/etc/squid/proxy_pass.txt",
                        help="password file")
    parser.add_argument("-u", "--unixsocket", type=str,
                        default='/var/run/redis/redis.sock',
                        help="unixsocket file")
    parser.add_argument("-e", "--expiration", type=int, default=7200,
                        help="seconds to expire session")
    ns = parser.parse_args()
    raise_error = False

    if ns.__dict__['thread'] < 1:
        print('-t need be one or more...')
        raise_error = True

    if not access(ns.__dict__['pass'], R_OK):
        print('-w file cant be read')
        raise_error = True

    unixsocket = ns.__dict__['unixsocket']
    expiration = ns.__dict__['expiration']

    if raise_error:
        parser.print_help()
        exit(1)

    label = ['CHANNEL', 'SRC', 'INTERFACE', 'PATH']
    passwd_list = read_dict(ns.__dict__['pass'])
    q = Queue()

    threads = []
    for tid in range(ns.__dict__['thread']):
        t = Thread(target=work_thread)
        t.start()
        threads.append(t)

    while True:
        try:
            value = stdin.readline()
            if not value:
                break
            q.put(value)
        except Full:
            pass

    for thread in threads:
        q.put('')

    for thread in threads:
        thread.join()
