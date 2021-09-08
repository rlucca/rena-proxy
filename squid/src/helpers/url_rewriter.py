#!/bin/python3

from proxy_controller import ProxyController
from sys import stdout
from sys import stdin
from sys import argv

generic_encoding = 'latin1'
# URL e traco
label = ['CHANNEL', 'URL', 'TRACO']

def process_line(main, fallback):
    try:
        line = stdin.readline().encode(generic_encoding)
        if not line:
            return 1
        clean_line = line.strip()
        if not clean_line:
            return 0
        data = dict(zip(label, clean_line.split(b' ')))
        scheme, domain, path = main.split_url(data['URL'])
        if main.get_suffix() in domain:
            new_domain = main.undo_redirect(data['URL'], domain)
            if new_domain:
                stdout.write('OK rewrite-url="')
                stdout.write(new_domain.strip().decode(generic_encoding))
                stdout.write('"\n')
                stdout.flush()
                return 0
            elif b'/login' in path:
                # LATER checagem de usuario/senha aqui?
                #   negativo, o ponto correto eh na
                #   liberacao_ip sem a senha correta n
                #   deveria nem chegar nesse ponto!
                temp = main.extract_url_from_path(path, scheme)
                (ok, new_domain) = main.do_redirect(temp, domain)
                if new_domain:
                    stdout.write('OK status=302 url="')
                    decode = new_domain.strip().decode(generic_encoding)
                    stdout.write(decode)
                    stdout.write('"\n')
                    stdout.flush()
                    return 0
        # url nao contem a nova linha entao insiro-a, no final
        sent_url = data['URL'].strip().decode(generic_encoding)
        if domain == main.get_suffix() and fallback:
            # fallback: /login?url=xxx
            sent_url += fallback
        stdout.write(sent_url)
    except EOFError:
        return 1
    except:
        stdout.write("ERR log=\"unknown error\"")
    stdout.write('\n')
    stdout.flush()
    return 0


if __name__ == '__main__':

    if len(argv) != 3:
        stdout.write("ERR log=\"informe um sufixo p/ o servidor e destino em caso de erro!\"\n")
        stdout.flush()
        exit(2)

    fallback = argv[2].strip()
    main = ProxyController(argv[1].encode(generic_encoding),
                           database_path='/etc/squid/dbs')

    while True:
        if process_line(main, fallback) > 0:
            break
        else:
            continue
