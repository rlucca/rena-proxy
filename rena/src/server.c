#include "global.h"
#include "server.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct server {
    SSL_CTX *server_context;
} server_t;

static SSL_CTX *create_ssl_context(struct rena *rena)
{
    const char *public = NULL;
    const char *private = NULL;
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

    if (ctx == NULL)
    {
        perror("SSL_CTX_new");
        ERR_print_errors_fp(stderr);
        do_log(LOG_ERROR, "ssl context failed");
        return NULL;
    }

    config_get_certificate_file(&rena->config, &public);
    config_get_certificate_key(&rena->config, &private);

    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, public, SSL_FILETYPE_PEM) <= 0
        || SSL_CTX_use_PrivateKey_file(ctx, private, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        do_log(LOG_ERROR, "ssl context failed");
        SSL_CTX_free(ctx);
        return NULL;
    }

    return ctx;
}

struct server *server_init(struct rena *rena)
{
    SSL_load_error_strings();
    SSL_library_init();

    rena->server = calloc(1, sizeof(struct server));
    rena->server->server_context = create_ssl_context(rena);

    if (rena->server->server_context == NULL)
    {
        EVP_cleanup();
        return NULL;
    }

    do_log(LOG_DEBUG, "not implemented yet");
    return NULL;
}

void server_destroy(struct rena *rena)
{
    do_log(LOG_DEBUG, "not implemented yet");
    SSL_CTX_free(rena->server->server_context);
    EVP_cleanup();

    free(rena->server);
    rena->server = NULL;
}
