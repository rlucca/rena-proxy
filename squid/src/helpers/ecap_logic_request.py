#!/bin/python3

# ProxyController eh usado instanciado na parte C
# e passado nas funcoes aqui criadas sempre como
# primeiro parametro!
# Em caso de substituicao mantenha o nome: ProxyController
from proxy_controller import ProxyController
import traceback
import sys

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
    pass


def adapt_header(state, name, value):
    try:
        # Nao precisamos testar pelo Content-Length
        # por causa que o mesmo somente se aplica
        # as modificacoes do corpo da requisicao,
        # dessa forma podemos nao remove-lo...

        # Temos um problema aqui que nem todos os
        # headers sao "editaveis" por exemplo se
        # na requisicao mexermos no Host. Terminaremos
        # enviando dois headers com mesmo valor no
        # melhor caso, por isso iremos procurar nosso
        # unico alvo!

        if name == b'Referer':
            pc = state['pc']
            _, domain, _ = pc.split_url(value)
            url = pc.undo_redirect(value, domain)
            # LATER seria problema se tem a porta?
            if url:
                return (True, url)

        return (False, None)

    except Exception as e:
        t, v, k = sys.exc_info()
        print('Exception: {} -=-> {}\nvalue [{}]'.format(
            e,
            traceback.format_exception(t, v, k), value))
        return (False, None)


def adapt_body(state, body_text):
    return None


def adapt_end(state):
    return None
