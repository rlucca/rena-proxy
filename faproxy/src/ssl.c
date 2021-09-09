#include "globals.h"
#include <openssl/err.h>
#include <unistd.h>

SSL_CTX *init_server_ctx()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = TLS_server_method();  /* create new server-method instance */
    if ( method == NULL )
    {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return ctx;
}

SSL_CTX *init_ssl(const char *cert, const char *key)
{
    SSL_CTX *ctx = NULL;

    SSL_library_init(); /* always return 1 */

    ctx = init_server_ctx();
    if (ctx == NULL)
        return NULL;

    /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        SSL_CTX_free(ctx);
        return NULL;
    }

    return ctx;
}

SSL *init_ssl_conn(int fd)
{
    SSL *ret = SSL_new(events.ssl_ctx); /* get new SSL state with context */
    SSL_set_fd(ret, fd);                /* set connection socket to SSL state */
    return ret;
}

int accept_ssl_conn(SSL *ssl)
{
    int ret;
    if (ssl == NULL)
    {
        DEBUG("SSL nulled");
        return -1;
    }

    ret = SSL_accept(ssl);
    if (ret < 0)
    {
        errno = SSL_get_error(ssl, ret);
        if (errno == SSL_ERROR_WANT_READ || errno == SSL_ERROR_WANT_WRITE)
        {
            DEBUG("ACCEPT: errno want_read or want_write");
            ret = SSL_accept(ssl);
        }

	if (ret < 0)
	{
            ERR_print_errors_fp(stderr);
        }
    }

    return ret;
}

void destroy_ssl_conn(SSL *ssl)
{
    int fd;

    if (ssl == NULL)
    {
        return ;
    }
    
    fd = SSL_get_fd(ssl);
    SSL_free(ssl);
    close(fd);
}
