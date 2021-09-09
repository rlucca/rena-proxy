#pragma once

SSL_CTX *init_server_ctx();
SSL_CTX *init_ssl(const char *cert, const char *key);

SSL *init_ssl_conn(int fd);
int accept_ssl_conn(SSL *ssl);
void destroy_ssl_conn(SSL *ssl); // destroy fd too
