#include "global.h"
#include "server.h"

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct server {
    SSL_CTX *server_context;
    int normalfd;
    int securefd;
} server_t;

static SSL_CTX *create_ssl_context(struct rena *rena)
{
    const char *public = NULL;
    const char *private = NULL;
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

    if (ctx == NULL)
    {
        do_log(LOG_ERROR, "ssl_context_new() failed -- %m");
        ERR_print_errors_fp(stderr);
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
        do_log(LOG_ERROR, "ssl_ctx_use_*_file() failed -- %m");
        SSL_CTX_free(ctx);
        return NULL;
    }

    return ctx;
}

static int create_socket(struct sockaddr_in6 *sa, int port)
{
    int ret=-1;
    int on=1;

    if ((ret = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
    {
        do_log(LOG_ERROR, "socket failed -- %m");
        return -1;
    }

    if (setsockopt(ret, SOL_SOCKET, SO_REUSEADDR,
                (void *)&on,sizeof(on)) < 0)
    {
        do_log(LOG_ERROR, "setsockopt failed -- %m");
        close(ret);
        return -1;
    }

    sa->sin6_port   = htons(port);

    if (bind(ret, (struct sockaddr *)sa, sizeof(*sa)) < 0)
    {
        do_log(LOG_ERROR, "bind() failed to port [%d] -- %m", port);
        close(ret);
        return -1;
    }

    if (listen(ret, SOMAXCONN) < 0)
    {
        do_log(LOG_ERROR, "listen() failed -- %m");
        close(ret);
        return -1;
    }

    return ret;
}

static void create_serving(struct rena *rena)
{
    const char *address = NULL;
    char ip[MAX_STR];
    int ports[2] = { -1, -1 };
    struct sockaddr_in6 sa;

    rena->server->normalfd = -1;
    rena->server->securefd = -1;

    config_get_server_address(&rena->config, &address);
    config_get_server_port_http(&rena->config, &ports[0]);
    config_get_server_port_https(&rena->config, &ports[1]);

    snprintf(ip, sizeof(ip), "%s%s",
                    (strchr(address, ':')==NULL)?"::FFFF:":"",
                    address);

    if (ports[0] < 0 || ports[0] > UINT16_MAX
        || ports[1] < 0 || ports[1] > UINT16_MAX)
    {
        do_log(LOG_ERROR, "port out of range");
        return ;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin6_family = AF_INET6;

    if (inet_pton(AF_INET6, ip, &(sa.sin6_addr)) <= 0)
    {
        do_log(LOG_ERROR,
               "failed to transform ip [%s] to binary form",
               ip);
        return ;
    }

    rena->server->normalfd = create_socket(&sa, ports[0]);
    rena->server->securefd = create_socket(&sa, ports[1]);
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

    create_serving(rena);

    if (rena->server->normalfd < 0
            || rena->server->securefd < 0)
    {
        EVP_cleanup();
        return NULL;
    }

    return rena->server;
}

void server_destroy(struct rena *rena)
{
    do_log(LOG_DEBUG, "not implemented yet");
    close(rena->server->normalfd);
    close(rena->server->securefd);

    SSL_CTX_free(rena->server->server_context);
    EVP_cleanup();

    free(rena->server);
    rena->server = NULL;
}
