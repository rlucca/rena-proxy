#!/bin/env python3

import json
import hashlib
import re
from sys import argv


def all_entries(obj):

    s, o = obj
    ret = {}
    for entry in o['log']['entries']:

        if entry['response']['status'] == 0:
            continue

        text = entry['response']['content'].get('text')
        if not text:
            continue

        key = entry['request']['url']
        start = key.index('://')
        end = key[start+3:].index('/')
        if end <= 0:
            print('nao previsto: ', start, ' ', end)
            exit(1)
        if key.find(s, start, end+start+4) < 0:
            continue
        kt = re.sub(s, '', key, count=1, flags=re.IGNORECASE).encode('utf8')
        kg = hashlib.md5(kt).hexdigest()

        rt = re.sub(s, '', text, flags=re.IGNORECASE)
        rg = hashlib.md5(rt.encode('utf8')).hexdigest()

        ret[kg] = {'key': key, 'txt': rg}
    return ret


def compare_dict(o, t, verbose=False):

    def printer(vo, vt):
        if vt is None:
            print('MISSING: ' + vo['key'])
        else:
            if vt['txt'] == vo['txt']:
                if verbose:
                    print('EQUAL: ' + vo['key'])
            else:
                print('DIFFER: ' + vo['key'])


    already_in_o = []
    for key in o:
        already_in_o.append(key)
        printer(o[key], t.get(key))

    for key in t:
        if key in already_in_o:
            continue
        printer(t[key], o.get(key))


if __name__ == '__main__':

    if len(argv) != 5:
        print('Usage: %s suffix1 har suffix2 har' % argv[0])
        print('Compare entries/request/url with suffix as key and ' \
              'response size and text')
        exit(1)

    original = (argv[1], json.load(open(argv[2])))
    target = (argv[3], json.load(open(argv[4])))

    dio = all_entries(original)
    dit = all_entries(target)

    compare_dict(dio, dit)
