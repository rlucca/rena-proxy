# Squid

Arquivos para compilacao do squid em ambiente de desenvolvimento.

src/ecap/
	arquivos do adaptador em C / Python
src/helpers/
	arquivos que sao plugins de ajuda ao squid tanto para reescrever a url como autenticacao
tarball/
	estrutura de diretorios com os arquivos compilados ou nao que devem ser instalados para o correto funcionamento do squid
Makefile
	diversas funcoes


Dependencias que os helpers devem precisar:

	- python3
    - python3-dev
    - libecap3
    - libecap3-dev
