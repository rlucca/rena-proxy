#!/bin/python3

import re
from enum import Enum, auto


class BaseParser(object):

    def accept(self, heading):
        raise NotImplementedError

    def visit(self, visitor, args):
        raise NotImplementedError


class NoProxyParser(BaseParser):

    def __init__(self):
        self.db = set()

    def accept(self, heading):
        return b'no_proxy' == heading

    def visit(self, visitor, args):
        # print('visit no proxy parser:', args)
        if len(args) == 2:
            visitor.visit_no_proxy(args[1])
        else:
            raise NotImplementedError


class RenameDomainParser(BaseParser):

    def accept(self, heading):
        return b'proxy_domain_rename' == heading

    def visit(self, visitor, args):
        # print('visit rename domain parser:', args)
        if len(args) == 3:
            visitor.visit_rename_domain(args[1], args[2])
        else:
            raise NotImplementedError


class SearchBy(Enum):

    BY_NEVER_PROXY = auto()
    BY_COMING = auto()
    BY_GOING = auto()


class Sentinel(object):

    def __init__(self, data):
        self.__value = data
        self.__length = len(data)

    @property
    def value(self):
        return self.__value

    @value.setter
    def value(self, data):
        self.__value = data
        self.__length = len(data)

    def length(self):
        return self.__length

    def __repr__(self):
        return (f'Sentinel({self.__value!r})')


# a finalidade dessa funcao eh mudar o ordenamento para ser
# nao por valor da expressao regular, mas sim pelo seu padrao escrito
def lens(x):
    pattern = x[1].pattern
    total = len(pattern)
    for c in pattern:
        total += c
    return total


class ProxyModel(object):

    def __init__(self, database_path='dbs', filename_wildcard='*.txt'):
        self.db_path = database_path
        self.filenames_mask = filename_wildcard
        self.registered_parser = [
            NoProxyParser(),
            RenameDomainParser()]
        self.db_no_proxy_iter = None
        self.db_proxy_going_iter = None
        self.db_proxy_coming_iter = None
        self.minimal_avail = 0
        self.maximal_avail = 0

    def __repr__(self):
        return (f'ProxyModel({self.db_path!r}, '
                f'{self.filenames_mask!r})')

    def filename_location(self):
        from os.path import sep
        return self.db_path + sep + self.filenames_mask

    def split_buffer(self, buf):
        array = re.split(br'\s+', buf)
        return array if array[-1] else array[:-1]

    def traverse(self):
        from glob import glob
        self.db_no_proxy = dict()
        self.db_proxy_going = dict()
        self.db_proxy_coming = dict()
        for filename in glob(self.filename_location()):
            with open(filename, 'rb') as fd:
                for line in fd:
                    splited_line = self.split_buffer(line)
                    for parser in self.registered_parser:
                        if parser.accept(splited_line[0]):
                            parser.visit(self, splited_line)
        self.db_no_proxy_iter = sorted(self.db_no_proxy.values(),
                                       key=lens, reverse=True)
        self.db_proxy_going_iter = sorted(self.db_proxy_going.values(),
                                          key=lens, reverse=True)
        self.db_proxy_coming_iter = sorted(self.db_proxy_coming.values(),
                                           key=lens, reverse=True)
        del self.db_no_proxy
        del self.db_proxy_going
        del self.db_proxy_coming

    def clean(self, buf):
        return buf.replace(br'.', br'\.')

    def visit_no_proxy(self, domain):
        cleaned = self.clean(domain)
        value = re.compile(cleaned, re.IGNORECASE)
        self.db_no_proxy[cleaned] = (cleaned, value)
        if self.minimal_avail == 0 or self.minimal_avail > len(cleaned):
            self.minimal_avail = len(cleaned)
        if self.maximal_avail == 0 or self.maximal_avail < len(cleaned):
            self.maximal_avail = len(cleaned)

    def visit_rename_domain(self, old_domain, new_domain):
        key_going = self.clean(old_domain)
        key_going_cmp = re.compile(key_going, re.IGNORECASE)
        self.db_proxy_going[key_going] = (new_domain, key_going_cmp)
        if self.minimal_avail == 0 or self.minimal_avail > len(key_going):
            self.minimal_avail = len(key_going)
        if self.maximal_avail == 0 or self.maximal_avail < len(key_going):
            self.maximal_avail = len(key_going)

        key_coming = new_domain + b'$'
        key_coming_cmp = re.compile(key_coming, re.IGNORECASE)
        cln_domain = re.sub(b'[$^]', b'', old_domain)

        self.db_proxy_coming[key_coming] = (cln_domain, key_coming_cmp)
        if self.minimal_avail == 0 or self.minimal_avail > len(key_coming):
            self.minimal_avail = len(key_coming)
        if self.maximal_avail == 0 or self.maximal_avail < len(key_coming):
            self.maximal_avail = len(key_coming)

    def check_database(self):
        for (element, _) in self.db_no_proxy_iter:
            for (res, setup) in self.db_proxy_going_iter:
                match = re.compile(b'^' + setup.pattern, re.IGNORECASE)
                # vamos remover do no_proxy o protetor do ponto porque
                # no teste vai ser considerado um caracter a mais e,
                # dessa forma, nao bateria porque o texto tem o '\'
                element = element.replace(br'\.', b'.')
                if match.search(element):
                    s = "'{}' e '{}' estao nas duas listas"
                    print(s.format(element, res))
                if type(res) != bytes:
                    raise Exception("wrong type for res")
                if type(setup.pattern) != bytes:
                    raise Exception("wrong type for setup.pattern")

    def search(self, text_in_value, searchIn):
        if searchIn == SearchBy.BY_NEVER_PROXY:
            elems = self.db_no_proxy_iter
        elif searchIn == SearchBy.BY_GOING:
            elems = self.db_proxy_going_iter
        elif searchIn == SearchBy.BY_COMING:
            elems = self.db_proxy_coming_iter
        else:
            elems = None

        if elems:
            for ret in elems:
                if text_in_value.length() < self.minimal_avail:
                    break
                if ret[1].search(text_in_value.value):
                    yield ret

    def minimal_characters(self):
        return self.minimal_avail

    def maximal_characters(self):
        return self.maximal_avail
