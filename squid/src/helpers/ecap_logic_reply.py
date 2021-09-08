#!/bin/python3

# ProxyController eh usado instanciado na parte C
# e passado nas funcoes aqui criadas sempre como
# primeiro parametro!
# Em caso de substituicao mantenha o nome: ProxyController
from proxy_controller import ProxyController
import re
import traceback
import sys

hdr_domain_match = re.compile(br'domain=(.*?); ?',
                              re.IGNORECASE)
generic_encoding = 'latin1'
ADP_ST_UNKNOWN = 0
ADP_ST_OUTSIDE_TAG = 1
ADP_ST_INSIDE_TAG = 2
ADP_ST_INSIDE_ATTRIBUTE = 3
ADP_ST_INSIDE_COMMENT = 4

# State nas funcoes abaixo contem elementos que sao do servico
# como URI e (request|reply)_service. Sufixo eh um estado interno,
# mas ja eh disponibilizado e de responsabilidade do controller que
# eh disponibilizado dentro do state com o nome abreviado de 'pc'.
# Alem disso, qualquer dado colocado no State eh mantido ate depois
# da chamada do adapt_end ocorrer.
#
# A sequencia de chamadas segue como:
#   adapt_begin -> None
#   adapt_header -> (tem_valor_novo, bytes)
#           retornar True e vazio, faz com que o header seja apagado
#   adapt_body -> None ou bytes
#           bytes eh a string adaptada nova a ser utilizada
#   adapt_end -> None or bytes
#           bytes eh o valor restante que faltava a ser adaptado
#
# Se necessario coloque print! Entretanto, o print nao vai pro log
# do squid e sim pra saida padrao. Dessa forma, coloque o programa
# em modo nao demon.


def adapt_begin(state):
    # print("adapt_begin: ", state)
    state['html'] = False
    state['ignore_text'] = True
    state['adaptation'] = ADP_ST_UNKNOWN
    state['buffer'] = b''
    state['context_atrb'] = b''
    state['context_text'] = b''
    state['accum'] = b''
    state['accum_limit'] = 512


def adapt_header(state, name, value):
    try:
        if name == b'content-length':
            # removeremos o Content-Length porque iremos mudar o
            # conteudo tanto dos headers quanto do corpo das
            # mensagens. Isso pode causar implicacoes de
            # velocidade para o cliente...
            return (True, None)

        if name == b'content-type':
            if b'html' in value.lower():
                state['html'] = True
            return (False, None)

        pc = state['pc']

        try:
            if hdr_domain_match.search(value):
                suffix = b'domain=' + pc.suffix + b';'
                return (True, hdr_domain_match.sub(suffix, value))
        except TypeError:
            pass

        (flag, url) = pc.do_redirect(value)
        if flag:
            return (True, url)

        return (False, None)

    except Exception as e:
        t, v, k = sys.exc_info()
        print('Exception: {} -=-> {}\nvalue [{}]'.format(
            e,
            traceback.format_exception(t, v, k), value))
        return (False, None)


def partial_change(state, chunk, ignore=False):
    if ignore:
        return None

    if chunk.startswith(b'mailto:'):
        return None

    if not chunk:
        return None

    value = state['pc'].do_all_redirects(chunk)
    return value


def outside_parser(state, head):
    ignore = state['ignore_text']
    if head in b' \t\v\n\r\f<':
        ab = partial_change(state, state['context_text'], ignore)
        if ab:
            state['buffer'] += ab + head
        else:
            state['buffer'] += state['context_text'] + head
        state['context_text'] = b''
        if head in b'<':
            state['ignore_text'] = True
            state['adaptation'] = ADP_ST_INSIDE_TAG
    else:
        state['context_text'] += head


def inside_parser(state, head):
    state['buffer'] += head
    lsb = len(state['buffer'])
    if lsb < 2 or state['buffer'][-2] != b'\\':
        if head in b'-' and lsb >= 3                     \
                    and state['buffer'][-2] == ord(b'-') \
                    and state['buffer'][-3] == ord(b'!'):
            state['adaptation'] = ADP_ST_INSIDE_COMMENT
            state['stage'] = 0
        elif head in b'>':
            state['adaptation'] = ADP_ST_OUTSIDE_TAG
        elif head in b'=':
            state['adaptation'] = ADP_ST_INSIDE_ATTRIBUTE


def ignore_comment(state, head):
    state['buffer'] += head
    if state['stage'] in [0, 1] and head in b'-':
        state['stage'] += 1
    elif state['stage'] == 2 and head in b'>':
        state['adaptation'] = ADP_ST_OUTSIDE_TAG


def attribute_parser(state, head):
    delim = b' \t\v\n\r\f>'

    # print(head, state['context_atrb'])
    waiting = state.get('adaptation_wait', delim)
    if (len(state['buffer']) < 1 or state['buffer'][-1] != b'\\') and head in waiting:
        ab = partial_change(state, state['context_atrb'])
        if ab:
            state['buffer'] += ab + head
            state['ignore_text'] = False
        else:
            state['buffer'] += state['context_atrb'] + head
        state['context_atrb'] = b''
        state['adaptation'] = ADP_ST_INSIDE_TAG
        try:
            del state['adaptation_wait']
        except KeyError:
            pass
        return
    else:
        state['context_atrb'] += head

    if not state.get('adaptation_wait') and len(state['context_atrb']) == 1:
        if state['context_atrb'][0] in b'\'"':
            state['adaptation_wait'] = state['context_atrb']
        else:
            state['adaptation_wait'] = delim


# Deixe fora da funcao mesmo!
adaptation_functions = [
    (ADP_ST_UNKNOWN, outside_parser),
    (ADP_ST_OUTSIDE_TAG, outside_parser),
    (ADP_ST_INSIDE_TAG, inside_parser),
    (ADP_ST_INSIDE_COMMENT, ignore_comment),
    (ADP_ST_INSIDE_ATTRIBUTE, attribute_parser)
]


def incremental_adaptation(state, body_text):
    for head in body_text:
        found = False
        for (marker, fnc) in adaptation_functions:
            if state['adaptation'] == marker:
                fnc(state, bytes([head]))
                found = True
                break
        if not found:
            exit(6)
    return state['buffer']


def adapt_body(state, body_text, force=False):
    try:
        # zeramos o buffer pq podemos ser chamado diversas
        # vezes e somente a parte nova deve ser enviada...
        state['accum'] = state['accum'] + body_text

        if not state['html']:
            if not force:
                # Sim, javascript, css e etc vamos acumular ate o final
                # por culpa da jwatch...
                return b''

            ret = partial_change(state, state['accum'])
            state['buffer'] = b''
            state['accum'] = b''
            return ret

        if not force and len(state['accum']) < state['accum_limit']:
            return b''
        ret = incremental_adaptation(state, state['accum'])
        state['buffer'] = b''
        state['accum'] = b''
        return ret
        # return None or String changed!
    except Exception as e:
        t, v, k = sys.exc_info()
        print('Exception: {} -=-> {}'.format(
            e,
            traceback.format_exception(t, v, k)))
        return None


def adapt_end(state):
    try:
        if state['accum']:
            inp = state['accum']
            state['accum'] = b''
            ret = adapt_body(state, inp, force=True) \
                + state['context_atrb'] + state['context_text']
        else:
            ret = state['context_atrb'] + state['context_text']
        return ret if ret else None
    except Exception as e:
        t, v, k = sys.exc_info()
        print('Exception: {} -=-> {}'.format(
            e,
            traceback.format_exception(t, v, k)))
        return None
