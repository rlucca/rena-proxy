# Squid - Documentacao - POC

### - Configuração de novos arquivos:
   * Consulte a configuração de `module_directory` para saber qual diretorio tem quer ir. Normalmente estará sendo o "/etc/squid/dbs";
   * Cada arquivo no diretório será carregado e parseado desde que a extensão final seja `txt`;
   * Os arquivos mencionados permitem definir:
     - `proxy_domain_rename .example.org -example` ,
         - os sub-domínios irão ser proxiados e a substituíção será pelo segundo parâmetro;
             - Assumindo um dominio "example.info":
                 - `www.example.org` ficaria `www-example.example.info`;
                 - `sub-21.example.org` ficaria `sub-21-example.example.info`;
     - `no_proxy www.example.org` , o domínio informado não irá ser proxiado.
             - Ao receber o acesso a "www.example.org/teste", o mesmo terá o proxy removido.
   * Dominios com portas são aceitos;
   * Comportamento é *indeterminados* no caso de domínios com usuário/senha ou com path completo (http://www.example.org/teste/teste#secao2)

### - Consulta a configuração de maneira remota
  * O ezproxy lista toda a configuração quando um administrador está logado,
      dessa forma foi incluído uma adaptação para termos essa possibilidade.
  * Para acessar primeiro consulte a configuração do squid feita pela tag `http_access`. <br>Será algo como:
<pre># Allow cachemgr access from localhost and rnp
acl rnp src 200.130.19.0/24
http_access allow localhost manager
http_access allow rnp manager
http_access deny manager

</pre>
  * O acesso não se encontra protegido por senha nem nada, basta ser feito do ip indicado como gerenciador.
  * Na configuração acima, todo o range 200.130.19.0 será permitido acessar essa listagem, além do acesso permitido pelo localhost.
  * Sabendo de tudo isso e assumindo que o servidor tem um domínio "example.info", acesse "example.info/squid-internal-mgr/proxyList".
  * É possível mudar a configuração para permitir as demais listagens e até colocar senha em todas ou em algumas especificas.
    

### - Definição de usuário / senha do proxy
  * Por padrão o mesmo esta desabilitado e qualquer usuário não precisa de usuário ou senha para acessa-lo.
  * Entretanto, pode ser ativada seguindo os passos a seguir.
  * Primeiro localize o arquivo externo `liberacao_user_pass_threading.py`;
      - Esse arquivo permite a autenticação de usuários e senhas definidos por um arquivo.
      - O arquivo padrão é "/etc/squid/proxy_pass.txt" e cada linha deve ser o usuario seguido de 1 espaço seguido da senha utilizada pelo mesmo.
  * Essa autenticação é feita por uma liberação por ip para cada usuário usando um autenticador externo ao squid. Seguido de uma liberação por ip para o domínio de destino do servidor.
<pre>
external_acl_type libera_up children-startup=1 children-max=1 concurrency=200 cache=0 %SRC %MYADDR %PATH /etc/squid/liberacao_user_pass_threading.py -t 2
acl lup external libera_up                                                                           
acl liberado dstdomain squid.example.info
# Liberamos o acesso ao autenticador
http_access allow lup
# Liberamos o acesso ao destino
http_access allow liberado
</pre>
   * Além da configuração acima é necessário alterar `http_access allow all` para `http_access deny all`. Lembrando que essa normalmente é a última linha do mencionada no arquivo de configuração sobre a tag `http_access` e é ideal que seja dessa forma.


### - Templates

Ha 4 templates que devem ser alterados para outra url:
    * ERR_ACCESS_DENIED
    * ERR_CACHE_ACCESS_DENIED
    * ERR_CACHE_MGR_ACCESS_DENIED
    * ERR_TOO_BIG
