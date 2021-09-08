#!/bin/python3

import re
from proxy_model import ProxyModel
from proxy_model import SearchBy
from proxy_model import Sentinel


cached_results = dict()


def cache_results(func):
    def func_wrapper(*args, **kwargs):
        key = b'!'.join(args[1:])
        if key in cached_results.keys():
            return cached_results[key]

        ret = func(*args, **kwargs)
        if len(cached_results.keys()) > 10:
            from random import randint
            r = randint(1, len(cached_results.keys()))
            to_be_removed = list(cached_results.keys())[r - 1]
            cached_results.pop(to_be_removed)
        cached_results[key] = ret
        return ret
    return func_wrapper


class ProxyController(object):

    def __init__(self, sufixo, **kws):
        self.suffix = sufixo
        if self.suffix.startswith(b'.'):
            self.suffix = self.suffix[1:]
        self.suffixER = b'.' + self.suffix + br'(?:\:\d+)?$'
        self.suffixER = br'^(.*)' + self.suffixER.replace(b'.', br'\.')
        self.suffixMatch = re.compile(self.suffixER, re.IGNORECASE)
        self.transformMatch = re.compile(br'url=(.*?)$')
        self.schemeMatch = re.compile(br'^.+://')
        self.portMatch = re.compile(br':\d+$')
        self.model = ProxyModel(**kws)
        self.model.traverse()

    def __repr__(self):
        return (f'ProxyController({self.suffix!r}, '
                + 'database_path={self.model.db_path!r}, '
                + 'filename_wildcard={self.model.filenames_mask!r})')

    def extract_url_from_path(self, path, scheme=b'http'):
        try:
            url = self.transformMatch.findall(path)[0]
            prefix = b''
            if not self.schemeMatch.match(url):
                if not url.startswith(b':'):
                    prefix = scheme + b'://'
                else:
                    prefix = scheme
            return prefix + url
        except Exception:
            pass
        return None

    def do_redirect(self, transform_url, real_suffix=None):
        # vamos substituir o parametro vazio pelo sufixo conhecido...
        if not real_suffix:
            real_suffix = self.get_suffix()
        schema, transform_domain, query = self.split_url(transform_url)
        # E a porta no split? A porta eh mantida no dominio pra casar por
        # expressao regular e removida no ir e na volta ser inserida...
        # Ver exemplo s0S.
        if not schema:
            schema = b'http:'

        # URL pedida ja contem proxy?
        subdomain = self.suffixMatch.search(transform_domain)
        if subdomain:
            # se recebermos uma url com o nosso prefixo...
            # vamos descartar o prefixo junto de porta e colocar
            # o de acesso base do servidor ou o configurado...
            new_subdomain = subdomain.groups()[0] + b'.' + real_suffix
            return (True, schema + b'//' + new_subdomain + query)

        # vou na base (a mesma eh mantida em lower case)
        d = Sentinel(transform_domain)

        # LATER se houver necessidade, da pra colocar a url toda tb...
        for _ in self.model.search(d, SearchBy.BY_NEVER_PROXY):
            # ver comentario no ultimo return
            return (False, transform_url)

        for (Vref, Vmatch) in self.model.search(d, SearchBy.BY_GOING):
            # Se ocorrer que alguma expressao re-escreveu a URL,
            # mas permaneceu a porta do lado direito. Para nao quebrar a
            # URL, retiramos a porta. A mesma nao voltara...
            full_domain = self.portMatch.sub(b'', Vmatch.sub(Vref, d.value))
            return (True,
                    schema + b'//' + full_domain + b'.' + real_suffix + query)

        # As urls nao cadastradas tem o proxy removido e o usuario
        # acessa elas sem o proxy. Caso seja interessante dar erro,
        # basta colocar None aqui. Isso causara um erro com a
        # seguinte mensagem: The request or reply is too large.
        # Eh possivel direcionar para outro erro com None ou
        # colocar uma pagina default aqui...
        return (False, transform_url)

    def do_all_redirects(self, data):
        if b'//' in data and b'.' in data and b' ' not in data:
            ref = self.split_url(data)
            ref = ref[1]
        else:
            ref = data

        wd = Sentinel(ref)

        # vamos descartar do work data todos never proxy...
        for (_, Vmatch) in self.model.search(wd, SearchBy.BY_NEVER_PROXY):
            wd.value = re.sub(Vmatch, b'', wd.value)

        # vamos buscar e substituir os restantes conforme esperado!
        for (Vref, Vmatch) in self.model.search(wd, SearchBy.BY_GOING):
            data = Vmatch.sub(Vref + b'.' + self.get_suffix(), data)
            wd.value = Vmatch.sub(b'', wd.value)

        return data

    # Durante a re-escrita a porta do proxy eh "perdida", nao achamos
    # isso um erro que vale a pena ser corrigido ja que a porta pode
    # ser incluida no rename...
    @cache_results
    def undo_redirect(self, uri, old_domain):
        coming_domain = self.suffixMatch.match(old_domain)
        # Checamos o suffixo e criamos um grupo do alias...
        if coming_domain and len(coming_domain.groups()) == 1:
            undo_domain_str = Sentinel(coming_domain.group(1))
            for (Vref, Vmatch) in self.model.search(undo_domain_str,
                                                    SearchBy.BY_COMING):
                full_domain = Sentinel(Vmatch.sub(Vref, undo_domain_str.value))
                for _ in self.model.search(full_domain,
                                           SearchBy.BY_GOING):
                    return re.sub(old_domain, full_domain.value, uri)
        return None

    def get_suffix(self):
        return self.suffix

    def split_url(self, uri):
        # Alguns casos possiveis:
        #   http://www.example.com
        #   //www.example.com
        #   /index?tim=1
        #   /moo#_123
        scheme_end = uri.find(b'//')
        if scheme_end >= 0:
            scheme = uri[:scheme_end].lower()
        else:
            scheme = b''
            scheme_end = -2
        uri = uri[scheme_end + 2:]
        # Casos possiveis de fim de dominio:
        # http://www.example.com/login
        # http://www.example.com?q=42
        # http://www.example.com#topic1
        domain_end = self._find_any_of(b'/?#', uri)
        domain = uri[:domain_end]
        # Ha possibilidade de vir uma url no estilo?
        # http://user1:pwd@www.example.com .
        # Aparentemente esse caso nao eh falado na RFC.
        start_domain = domain.find(b'@')
        if start_domain >= 0:
            domain = domain[start_domain + 1:]
        path = uri[domain_end:]
        return scheme, domain, path

    def _find_any_of(self, to_find, target):
        max_elem = 0
        for (POS, ch) in enumerate(target):
            max_elem = POS
            if ch in to_find:
                return POS
        return max_elem + 1
