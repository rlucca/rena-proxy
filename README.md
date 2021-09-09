# rena-proxy

Um proxy para funcionar de maneira similar ao EZ proxy.

## Historia

Ano passado fiz uma adaptacao no squid que no caso nao ficou usavel e esta
sendo disponibilizada como esta no diretorio squid. Essa forma pode ser usada
com o squid sem a necessidade de nenhuma mudanca no mesmo ja que utiliza o
sistema de adaptacao de conteudo do mesmo.

Mais detalhes no referido diretorio: squid.


Um outro teste foi feito usando codigo puramente python em flask. Essa
solucao nao foi completa por causa que realizei testes transformando as strings
para listas e descartei continuar com um micro-servico para atender o que
gostaria.

O codigo desse desenvolvimento esta em: flask.


O codigo no diretorio faproxy contem uma implementacao que nao foi totalmente
testada. Ela teve o desenvolvimento parado iniciando a parte do https. No
entanto, com a libev sendo usada para fazer a gerencia dos sockets diversas
vezes tive atritos com ela que tive que aprender como fazer o que gostaria na
sua forma especificada e, em algumas vezes, de forma criativa... Em teoria,
lembro de so ter terminado os testes em HTTP puro e o HTTPS enloquecia.

Mais detalhes no diretorio: faproxy.


## Lista de pedidos a serem atendidos

- Proxy HTTP e HTTPs
- Substituicao de conteudo em ambos os casos
    * Um texto arbitrario que seja indicacao de algo que o proxy da acesso
      deve ser substituido mesmo que o "site" nao seja aquele da URL.
      Ex.: Ao acessar a Elsevier e a mesma ter um link para baseado em DOI,
           esse ultimo deve ser substituido tambem.
- Autenticacao por IP, HTTP Basic e por certificado
- Logs de acesso personalizavel
- Configurar interface de entrada e saida
- Manter a velocidade de acesso em comparacao ao atual
- Manter formas configuracao de adaptacao de conteudo similares a atual
    * Isso inclui ainda a possibilidade de criacao de cookie para ir no
      servidor ou para o cliente como forma de registrar ultimo acesso.
- A quantidade de memoria e processador deve se manter estavel


Cabe chamar atencao que a solucao implementada com o squid com excessao das
duas ultimas, atende todos os requisitos acima.
