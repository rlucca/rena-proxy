#!/bin/python3

from proxy_controller import ProxyController
from sys import stdout
from sys import stdin
from sys import argv
from queue import Queue, Full
from threading import Thread
import asyncio

generic_encoding = 'latin1'
# URL e traco
label = ['CHANNEL', 'URL', 'TRACO']
threads = []
q = Queue()


def send_answer(msg):
    stdout.write(msg)
    stdout.flush()


async def send_rewrite(ch, domain):
    decode = domain.strip().decode(generic_encoding)
    send_answer(f'{ch} OK rewrite-url="{decode}"\n')


async def send_redirect(ch, domain):
    decode = domain.strip().decode(generic_encoding)
    send_answer(f'{ch} OK status=302 url="{decode}"\n')


async def send_normal(ch, url, is_suffix):
    decode = url.strip().decode(generic_encoding)
    if is_suffix and fallback:
        # fallback: /login?url=xxx
        decode += fallback
    send_answer(f'{ch} {decode}\n')


async def get_next_element():
    return q.get()


async def undo_redirect(main, url, domain):
    return main.undo_redirect(url, domain)


async def do_redirect(main, url, domain):
    return main.do_redirect(url, domain)


async def do_work(main, loop):
    while True:
        try:
            item = await get_next_element()
            if not item:
                break
            clean_line = item.strip()
            if not clean_line:
                q.task_done()
                continue

            data = dict(zip(label, clean_line.split(b' ')))
            scheme, domain, path = main.split_url(data['URL'])
            channel = data['CHANNEL'].decode(generic_encoding)
            if main.get_suffix() in domain:
                new_domain = await undo_redirect(main, data['URL'], domain)
                if new_domain:
                    await send_rewrite(channel, new_domain)
                    q.task_done()
                    continue
                elif b'/login' in path:
                    # LATER checagem de usuario/senha aqui?
                    #   negativo, o ponto correto eh na
                    #   liberacao_ip sem a senha correta n
                    #   deveria nem chegar nesse ponto!
                    temp = main.extract_url_from_path(path, scheme)
                    (ok, new_domain) = await do_redirect(main, temp, domain)
                    if new_domain:
                        await send_redirect(channel, new_domain)
                        q.task_done()
                        continue
            # url nao contem a nova linha entao insiro-a, no final
            await send_normal(channel, data['URL'], domain == main.get_suffix())
            q.task_done()
        except:
            raise
            exit(1)


def work_thread():
    # ns/fallback estao no escopo global...
    main = ProxyController(ns.__dict__['suffix'].encode(generic_encoding),
                           database_path=ns.__dict__['database'])
    loop = asyncio.new_event_loop()
    loop.run_until_complete(do_work(main, loop))


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--thread", type=int, default=4,
                        help="number of working threads (4)")
    parser.add_argument("-s", "--suffix", type=str,
                        help="suffix to be used (squidN.example.com)")
    parser.add_argument("-f", "--fallback", type=str,
                        default='/login?url=http://www.example.com',
                        help="fallback redirect (/login?url=http://www.example.com)")
    parser.add_argument("-d", "--database", type=str,
                        default='/etc/squid/dbs',
                        help="database directory (/etc/squid/dbs)")
    ns = parser.parse_args()
    raise_error = False

    if ns.__dict__['thread'] < 1:
        print('-t need be one or more...')
        raise_error = True

    if not ns.__dict__['suffix']:
        print('-s not informed!')
        raise_error = True

    if not ns.__dict__['fallback']:
        print('-f not informed or empty!')
        raise_error = True

    if raise_error:
        parser.print_help()
        exit(1)

    fallback = ns.__dict__['fallback'].strip()

    for tid in range(ns.__dict__['thread']):
        t = Thread(target=work_thread)
        t.start()
        threads.append(t)

    while True:
        try:
            value = stdin.readline().encode(generic_encoding)
            if not value:
                break
            q.put(value)
        except Full:
            pass

    for thread in threads:
        q.put('')

    for thread in threads:
        thread.join()
