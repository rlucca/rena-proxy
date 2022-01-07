#include "global.h"
#include "server.h"
#include "proc.h"
#include "task_manager.h"
#include "clients.h"

#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>

typedef struct server {
    SSL_CTX *server_context;
    int normalfd;
    int securefd;
    int pollfd;
    int signalfd;
} server_t;

static int poll_init()
{
    int ret = epoll_create1(EPOLL_CLOEXEC);
    if (ret < 0)
    {
        do_log(LOG_ERROR, "epoll_create failed -- %m");
        return -1;
    }

    return ret;
}

int server_notify(struct rena *rena, int op, int fd, int submask)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLONESHOT | submask;
    ev.data.fd = fd;
    if (epoll_ctl(rena->server->pollfd, op, fd, &ev) < 0)
    {
        char buf[MAX_STR];
        proc_errno_message(buf, sizeof(buf));
        do_log(LOG_ERROR, "epoll_ctl failed on [%d]: %s", fd, buf);
        return -1;
    }

    return 0;
}

int server_dispatch(struct rena *rena)
{
    #define MAX 1024
    #define TIMEOUT_MS 300
    struct epoll_event evs[MAX];
    int nfds = -1;
    int qty = 4 + clients_quantity(rena->clients);
    int timeout = (qty >= MAX) ? TIMEOUT_MS : -1;

    do_log(LOG_DEBUG,"waiting for io [%d]", qty);
    nfds = epoll_wait(rena->server->pollfd, evs, MAX, timeout);
    do_log(LOG_DEBUG,"received %d fds", nfds);

    if (nfds < 0)
    {
        char buf[MAX_STR];
        proc_errno_message(buf, sizeof(buf));
        do_log(LOG_ERROR, "epoll_wait failed: %s", buf);
        return -1;
    }

    // Todos os itens precisam ser recolocados na fila como MOD
    for (int n=0; n < nfds; n++)
    {
        if ((evs[n].events & EPOLLOUT) == EPOLLOUT)
        {
            task_type_e tte = TT_WRITE;
            do_log(LOG_DEBUG,"pushing write task");
            if (rena->server->normalfd == evs[n].data.fd)
                tte = TT_NORMAL_WRITE;
            else if (rena->server->securefd == evs[n].data.fd)
                tte = TT_SECURE_WRITE;
            else if (rena->server->signalfd == evs[n].data.fd)
                tte = TT_SIGNAL_WRITE;
            task_manager_task_push(rena, evs[n].data.fd, tte);
        } else if ((evs[n].events & EPOLLIN) == EPOLLIN)
        {
            task_type_e tte = TT_READ;
            do_log(LOG_DEBUG,"pushing read task");
            if (rena->server->normalfd == evs[n].data.fd)
                tte = TT_NORMAL_READ;
            else if (rena->server->securefd == evs[n].data.fd)
                tte = TT_SECURE_READ;
            else if (rena->server->signalfd == evs[n].data.fd)
                tte = TT_SIGNAL_READ;
            task_manager_task_push(rena, evs[n].data.fd, tte);
        } else
        {
            do_log(LOG_ERROR, "oh noo!");
            abort();
        }
    }

    return 0;
    #undef MAX
    #undef TIMEOUT_MS
}

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

    if (server_fd_nonblock(rena->server->normalfd))
    {
        do_log(LOG_ERROR,
               "failed to change normal socket to asynchronous");
    }

    if (server_fd_nonblock(rena->server->securefd))
    {
        do_log(LOG_ERROR,
               "failed to change secure socket to asynchronous");
    }
}

struct server *server_init(struct rena *rena)
{
    SSL_load_error_strings();
    SSL_library_init();

    rena->server = calloc(1, sizeof(struct server));
    rena->server->pollfd = poll_init();
    if (rena->server->pollfd < 0)
    {
        close(rena->server->pollfd);
        free(rena->server);
        rena->server = NULL;
        return NULL;
    }

    rena->server->server_context = create_ssl_context(rena);
    if (rena->server->server_context == NULL)
    {
        EVP_cleanup();
        close(rena->server->pollfd);
        free(rena->server);
        rena->server = NULL;
        return NULL;
    }

    int error = 0;

    create_serving(rena);
    if (rena->server->normalfd < 0
            || rena->server->securefd < 0)
    {
        error = 1;
    }

    if (error || server_notify(rena, EPOLL_CTL_ADD,
                               rena->server->normalfd, EPOLLIN) < 0)
    {
        error = 1;
    }

    if (error || server_notify(rena, EPOLL_CTL_ADD,
                               rena->server->securefd, EPOLLIN) < 0)
    {
        error = 1;
    }

    if (error || proc_signal_block() < 0)
    {
        error = 1;
    }

    if (error
        || (rena->server->signalfd = proc_create_signalfd()) < 0
        || server_notify(rena, EPOLL_CTL_ADD,
                         rena->server->signalfd, EPOLLIN) < 0)
    {
        error = 1;
    }

    if (error)
    {
        EVP_cleanup();
        close(rena->server->pollfd);
        close(rena->server->normalfd);
        close(rena->server->securefd);
        close(rena->server->signalfd);
        free(rena->server);
        rena->server = NULL;
    }

    return rena->server;
}

void server_destroy(struct rena *rena)
{
    struct server *server = rena->server;
    if (!server)
    {
        return ;
    }

    close(server->signalfd);
    close(server->normalfd);
    close(server->securefd);
    close(server->pollfd);

    SSL_CTX_free(server->server_context);
    EVP_cleanup();

    free(server);
    rena->server = NULL;
}

int server_fd_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
}
