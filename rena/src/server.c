#define _GNU_SOURCE
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
#include <netdb.h>


typedef struct server {
    SSL_CTX *server_context;
    SSL_CTX *client_context;
    struct sockaddr_in6 address;
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

static int server_notify2(struct rena *rena, int op, int fd, int submask)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = submask|EPOLLERR|EPOLLHUP|EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(rena->server->pollfd, op, fd, &ev) < 0)
    {
        text_t buf;
        int error = proc_errno_message(&buf);
        //do_log(LOG_ERROR, "epoll_ctl failed on [%d]: %s", fd, buf.text);
        if (error == EBADF)
            return -1;
        if (error != EEXIST)
            return -2;
    }

    return 0;
}

int server_notify(struct rena *rena, int fd, int submask)
{
    int rn = -1;
    if (fd < 0)
        return -1;

    submask = submask | EPOLLONESHOT;
    rn = server_notify2(rena, EPOLL_CTL_MOD, fd, submask);
    if (rn < 0)
    {
        if (rn == -1)
            return -1;

        rn = server_notify2(rena, EPOLL_CTL_ADD, fd, submask);
        if (rn < 0)
            return -2;
    }

    return 0;
}

static task_type_e fixing_task_type(struct rena *rena, int fd,
                             task_type_e type)
{
    task_type_e tte = type + 2;
    if (rena->server->normalfd == fd)
        return tte;

    tte += 2;
    if (rena->server->securefd == fd)
        return tte;

    return type;
}

void server_tm_push(struct rena *rena, int fd, int tte)
{
    task_manager_task_push(rena, fd, tte);
}

static void handle_signals(struct rena *rena, int fd)
{
    int s = -1;
    while (fd > 0 && (s = proc_receive_signal(fd)) >= 0)
    {
        do_log(LOG_DEBUG, "Received signal [%d]", s);

        if (proc_terminal_signal(s))
        {
            do_log(LOG_DEBUG, "set forced exit to 1");
            rena->forced_exit = 1;
        }

        if (proc_respawn_signal(s) && rena->forced_exit == 0)
        {
            do_log(LOG_ERROR, "a co-worker died! lets die as family :'(");
            rena->forced_exit = 1;
        }

        if (proc_starting_task_signal(s))
        {
            task_manager_new_thread(rena);
        }
    }
}

int server_dispatch(struct rena *rena)
{
    #define MAX 1024
    #define TIMEOUT_MS 300
    struct epoll_event evs[MAX];
    int nfds = -1;
    int timeout = TIMEOUT_MS;
    int signalcheck = 0;

    nfds = epoll_wait(rena->server->pollfd, evs, MAX, timeout);

    if (nfds < 0)
    {
        text_t buf;
        proc_errno_message(&buf);
        do_log(LOG_ERROR, "epoll_wait failed: %s", buf.text);
    } else {
        // All itens need to be re-queued, do not forget!!!
        for (int n=0; n < nfds; n++)
        {
            int fd=evs[n].data.fd;
            if (rena->server->signalfd == fd)
                signalcheck = fd;
            else if ((evs[n].events & EPOLLOUT))
            {
                task_type_e tte = fixing_task_type(rena, fd, TT_WRITE);
                do_log(LOG_DEBUG,"pushing write task for fd:%d", fd);
                server_tm_push(rena, fd, tte);
            } else if ((evs[n].events & EPOLLIN))
            {
                task_type_e tte = fixing_task_type(rena, fd, TT_READ);
                do_log(LOG_DEBUG,"pushing read task for fd:%d",fd);
                server_tm_push(rena, fd, tte);
            } else if ((evs[n].events & EPOLLHUP))
            {
                do_log(LOG_DEBUG,"pushing HUP task for client fd:%d", fd);
                server_tm_push(rena, fd, TT_WRITE);
            } else
            {
                do_log(LOG_ERROR, "event handle not set to [%d] for fd:%d",
                       evs[n].events, fd);
                abort();
            }
        }
    }

    handle_signals(rena, signalcheck);
    clients_alive(rena, rena->clients);
    return (nfds < 0) ? -1 : 0;
    #undef MAX
    #undef TIMEOUT_MS
}

static int fd_set_common_flags(int fd)
{
    int perror = 0;
    int on = 1;
    struct linger L;
    L.l_onoff = 1;
    L.l_linger = 5;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                (void *)&on,sizeof(on)) < 0)
    {
        perror = errno;
    }

    if (perror == 0
            && setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
                            (void *)&on,sizeof(on)) < 0)
    {
        perror = errno;
    }

    if (perror == 0
            && setsockopt(fd, SOL_SOCKET, SO_LINGER,
                            (char *) &L, sizeof(L)) < 0)
    {
        perror = errno;
    }

    if (perror != 0)
    {
        text_t buf;
        errno = perror;
        proc_errno_message(&buf);
        do_log(LOG_ERROR, "setsockopt failed -- %s", buf.text);
        proc_close(fd);
        return -1;
    }

    return 0;
}

static void server_ssl_ctx_options(SSL_CTX *ctx)
{
    if (!ctx)
        return ;

    // turn off certificate verification...
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    // appears that need set no compression when using http/2.0 compression
    SSL_CTX_set_options(ctx, SSL_OP_ALL
                             | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
                             | SSL_OP_NO_COMPRESSION
                             | SSL_OP_CIPHER_SERVER_PREFERENCE
#ifdef SSL_OP_PRIORITIZE_CHACHA
                             | SSL_OP_PRIORITIZE_CHACHA
#endif
                       );

    long mode = SSL_CTX_get_mode(ctx);
    SSL_CTX_set_mode(ctx, mode
                          | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
                          | SSL_MODE_ENABLE_PARTIAL_WRITE
                          | SSL_MODE_RELEASE_BUFFERS);
    SSL_CTX_clear_mode(ctx, SSL_MODE_AUTO_RETRY); // turn off AUTO RETRY
    do_log(LOG_DEBUG, "ssl setting mode [%lx -> %lx]",
           mode, SSL_CTX_get_mode(ctx));
}

static SSL_CTX *create_ssl_context_server(struct rena *rena)
{
    const char *public = NULL;
    const char *private = NULL;
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

    if (ctx == NULL)
    {
        do_log(LOG_ERROR, "ssl_context_server_new() failed -- %m");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    config_get_certificate_file(&rena->config, &public);
    config_get_certificate_key(&rena->config, &private);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, public, SSL_FILETYPE_PEM) <= 0
        || SSL_CTX_use_PrivateKey_file(ctx, private, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        do_log(LOG_ERROR, "ssl_ctx_use_*_file() failed -- %m");
        SSL_CTX_free(ctx);
        return NULL;
    }

    server_ssl_ctx_options(ctx);
    return ctx;
}

static SSL_CTX *create_ssl_context_client(struct rena *rena)
{
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (ctx == NULL)
    {
        do_log(LOG_ERROR, "ssl_context_client_new() failed -- %m");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    server_ssl_ctx_options(ctx);
    return ctx;
}

static int create_socket(struct sockaddr_in6 *sa)
{
    int ret=-1;

    if ((ret = socket(sa->sin6_family, SOCK_STREAM|SOCK_NONBLOCK, 0)) < 0)
    {
        text_t buf;
        proc_errno_message(&buf);
        do_log(LOG_ERROR, "socket failed -- %s", buf.text);
        return -1;
    }

    if (fd_set_common_flags(ret))
    {
        return -1;
    }

    return ret;
}

static int server_create_socket(struct sockaddr_in6 *sa, int port)
{
    int ret = create_socket(sa);

    if (ret < 0)
    {
        return -1;
    }

    sa->sin6_port   = htons(port);
    if (bind(ret, (struct sockaddr *)sa, sizeof(*sa)) < 0)
    {
        do_log(LOG_ERROR, "bind() failed to port [%d] -- %m", port);
        proc_close(ret);
        return -1;
    }

    if (listen(ret, SOMAXCONN) < 0)
    {
        do_log(LOG_ERROR, "listen() failed -- %m");
        proc_close(ret);
        return -1;
    }

    return ret;
}

int server_socket_for_client(struct rena *rena, void *address)
{
    struct addrinfo *info = address;
    struct sockaddr_in6 sa = rena->server->address;
    int ret;

    sa.sin6_family = info->ai_family;
    ret = create_socket(&sa);

    if (ret < 0)
    {
        return -1;
    }

    sa.sin6_port   = 0; // ANY PORT, we are local client starting
    if (bind(ret, &sa, sizeof(sa)) < 0)
    {
        text_t buf;
        proc_errno_message(&buf);
        do_log(LOG_ERROR, "failed to bind local client %s", buf.text);
        proc_close(ret);
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

    rena->server->normalfd = server_create_socket(&sa, ports[0]);
    rena->server->securefd = server_create_socket(&sa, ports[1]);
    rena->server->address = sa;
}

static void verify_server_and_create_signalfd(struct rena *rena)
{
    int error = 0;
    if (rena->server->normalfd < 0 || rena->server->securefd < 0)
    {
        error = 1;
    }

    if (error || server_notify2(rena, EPOLL_CTL_ADD,
                               rena->server->normalfd, EPOLLIN) < 0)
    {
        error = 1;
    }

    if (error || server_notify2(rena, EPOLL_CTL_ADD,
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
        || server_notify2(rena, EPOLL_CTL_ADD,
                         rena->server->signalfd, EPOLLIN) < 0)
    {
        error = 1;
    }

    if (error)
    {
        SSL_CTX_free(rena->server->server_context);
        SSL_CTX_free(rena->server->client_context);
        EVP_cleanup();
        proc_close(rena->server->pollfd);
        proc_close(rena->server->normalfd);
        proc_close(rena->server->securefd);
        proc_close(rena->server->signalfd);
        free(rena->server);
        rena->server = NULL;
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
        proc_close(rena->server->pollfd);
        free(rena->server);
        rena->server = NULL;
        return NULL;
    }

    rena->server->server_context = create_ssl_context_server(rena);
    if (rena->server->server_context == NULL)
    {
        EVP_cleanup();
        proc_close(rena->server->pollfd);
        free(rena->server);
        rena->server = NULL;
        return NULL;
    }

    rena->server->client_context = create_ssl_context_client(rena);
    if (rena->server->client_context == NULL)
    {
        SSL_CTX_free(rena->server->server_context);
        EVP_cleanup();
        proc_close(rena->server->pollfd);
        free(rena->server);
        rena->server = NULL;
        return NULL;
    }


    create_serving(rena);
    verify_server_and_create_signalfd(rena);
    return rena->server;
}

void server_destroy(struct rena *rena)
{
    struct server *server = rena->server;
    if (!server)
    {
        return ;
    }

    proc_close(server->signalfd);
    proc_close(server->normalfd);
    proc_close(server->securefd);
    proc_close(server->pollfd);

    SSL_CTX_free(server->server_context);
    SSL_CTX_free(server->client_context);
    EVP_cleanup();

    free(server);
    rena->server = NULL;
}

static int ignore_error(int error)
{
    if (error == EINTR || error == EINPROGRESS || error == EALREADY
        || error == ERESTART || error == EAGAIN || error == EWOULDBLOCK)
    {
        return 1;
    }

    return 0;
}

int server_receive_client(struct rena *rena, int fd, void **ssl)
{
    int new_fd = -1;

    if (!rena || !rena->server)
    {
        void *pointer = rena->server;
        if (!rena) pointer = rena;
        do_log(LOG_ERROR, "argument server cant be null %p",
               pointer);
        return -1;
    }

    new_fd = accept4(fd, NULL, NULL,
                     SOCK_CLOEXEC|SOCK_NONBLOCK);
    if (new_fd < 0)
    {
        text_t buf;
        int error = proc_errno_message(&buf);
        if (ignore_error(error))
          return -2;
        do_log(LOG_ERROR, "accept failed on [%d]: %s", fd, buf.text);
        return -1;
    }

    if (ssl)
    {
        *ssl = SSL_new(rena->server->server_context);
        if (*ssl == NULL)
        {
            proc_close(new_fd);
            do_log(LOG_ERROR,
                   "SSL_new failed: dropping new client [%d]",
                   new_fd);
            return -1;
        }

        if (SSL_set_fd(*ssl, new_fd) == 0)
        {
            proc_close(new_fd);
            do_log(LOG_ERROR,
                   "SSL_set_fd failed: dropping new client [%d]",
                   new_fd);
            return -1;
        }

        SSL_set_accept_state(*ssl);
    }

    return new_fd;
}

static int ssl_error(int fd, SSL *ssl, int error, int gerror)
{
    int err = SSL_get_error(ssl, error);
    int ret = 0;
    if (err == SSL_ERROR_NONE)
        return 0; // OK?!?
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_ACCEPT)
        return TT_READ;
    if (err == SSL_ERROR_WANT_WRITE)
        return TT_WRITE;
    ret = ERR_get_error();
    if (ret != 0)
    {
        do {
            char error_string[MAX_STR];
            ERR_error_string_n(ret, error_string, sizeof(error_string));
            do_log(LOG_ERROR,
                   "SSL ERROR: %d code %d %s",
                    err, ret, error_string);
        } while ((ret = ERR_get_error()));
    } else if (gerror != 0) {
        if (gerror != EPIPE && gerror != ECONNRESET)
            do_log(LOG_ERROR, "SSL (error): %d", err);
    }

    if (err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL)
    {
        int xerr = 0;
        socklen_t xerrlen = sizeof(xerr);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &xerr, &xerrlen);
        do_log(LOG_DEBUG, "ssl error SYSCALL socket error %d [errno %d]",
               xerr, gerror);
    }

    return (err == SSL_ERROR_ZERO_RETURN)?-1:-2;
}

int server_handshake_client(int fd, void *is_ssl)
{
    SSL *ssl = (void *) is_ssl;
    int r = SSL_do_handshake(ssl);
    if (!ssl || r == 1)
        return 0; // OK!
    return ssl_error(fd, ssl, r, errno);
}

int server_address_from_host(const char *host, void **out)
{
    struct addrinfo hints;
    struct addrinfo *result;
    int s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(host, NULL, &hints, &result);
    if (s != 0) {
        // maybe gai_strerror return a static string!
        do_log(LOG_ERROR, "DNS retornou erro [%s] para [%s]!",
               gai_strerror(s), host);
        return -1;
    }

    *out = result;
    return 0;
}

void server_address_set_port(void *address, int port)
{
    struct addrinfo *rp;

    for (rp = (struct addrinfo *) address;
         rp != NULL;
         rp = rp->ai_next)
    {
        struct sockaddr_in *ai =
            (struct sockaddr_in *) rp->ai_addr;
        ai->sin_port = htons(port);
    }
}

int server_read_client(int fd, void *is_ssl, text_t *out, int *retry)
{
    SSL *ssl = (void *) is_ssl;
    int r = -1;
    int ret = 0;
    int gerr = 0;
    errno = 0;
    if (!ssl)
    {
        r = read(fd, out->text, sizeof(out->text));
        gerr = errno;
        if (r < 0)
        {
            if (ignore_error(gerr) == 0)
            {
                ret = -1;
            } else {
                *retry = 1;
            }
        } else if (r == 0)
        {
            ret = -1;
        }
    } else {

        r = SSL_read(ssl, out->text, sizeof(out->text));
        gerr = errno;
        if (r <= 0)
        {
            ret = ssl_error(fd, ssl, r, gerr);
        }
    }

    do_log(LOG_DEBUG, "read fd:%d returned [%d] function ret [%d] "
                      "retry [%d] errno [%d] output [%lu]",
            fd, r, ret, *retry, gerr, sizeof(out->text));
    out->size = r;
    return ret;
}

int server_write_client(int fd, void *is_ssl, void *output, size_t *output_len,
                        int *retry)
{
    SSL *ssl = (void *) is_ssl;
    int r = -1;
    int ret = 0;
    int gerr = 0;
    if (!ssl)
    {
        r = write(fd, output, *output_len);
        gerr = errno;
        if (r < 0)
        {
            if (ignore_error(gerr) == 0)
            {
                ret = -1;
            } else {
                *retry = 1;
            }
        } else if (r == 0)
        {
            ret = -1;
        }
    } else {

        r = SSL_write(ssl, output, *output_len);
        gerr = errno;
        if (r <= 0)
        {
            ret = ssl_error(fd, ssl, r, gerr);
        }
    }

    do_log(LOG_DEBUG, "write fd:%d returned [%d] function ret [%d] "
                      "retry [%d] errno [%d] output [%lu]",
            fd, r, ret, *retry, gerr, *output_len);
    *output_len = r;
    return ret;
}

int server_set_client_as_secure(struct rena *rena, void *peer, const char *sni)
{
    SSL *ssl = SSL_new(rena->server->client_context);
    if (ssl == NULL)
    {
        do_log(LOG_ERROR, "problem creating client context");
        return 1;
    }

    clients_set_ssl(peer, ssl);

    if (sni)
    {
        do_log(LOG_DEBUG, "setting server name identification to [%s]", sni);
        SSL_set_tlsext_host_name(ssl, sni);
    }
    return 0;
}

int server_tcp_connection_done(int fd)
{
    int xerr = 0;
    socklen_t xerrlen = sizeof(xerr);
    int einval = 0;
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &xerr, &xerrlen) < 0)
    {
        einval = 1;
    }

    if (xerr != 0 || einval != 0)
    {
        do_log(LOG_DEBUG, "error [%d] completing tcp connection on fd [%d]!",
               einval + xerr, fd);
        return -1;
    }

    do_log(LOG_DEBUG, "fd:%d returned [%d]", fd, xerr);
    return 0;
}

static int connection_in_loop(struct addrinfo *target, int fd)
{
    // LATER port test, ....
    if (target->ai_family == AF_INET)
    {
        struct sockaddr_in *tmp=(struct sockaddr_in *) target->ai_addr;
        struct sockaddr_in tmp2;
        unsigned int tmp2sz = sizeof(tmp2);
        getsockname(fd, &tmp2, &tmp2sz);
        if(!memcmp(&tmp->sin_addr, &tmp2.sin_addr, sizeof(tmp->sin_addr)))
        {
            return 1;
        }
    }
    else if(target->ai_family == AF_INET6)
    {
        struct sockaddr_in6 *tmp=(struct sockaddr_in6 *) target->ai_addr;
        struct sockaddr_in6 tmp2;
        unsigned int tmp2sz = sizeof(tmp2);
        getsockname(fd, &tmp2, &tmp2sz);
        if(!memcmp(&tmp->sin6_addr, &tmp2.sin6_addr, sizeof(tmp->sin6_addr)))
        {
            return 1;
        }
    }

    return 0;
}

static int server_client_connect(struct rena *rena, void *peer)
{
    struct addrinfo *target = clients_get_userdata(peer);
    int vfd = clients_get_fd(peer);
    int ret;

    if (target == NULL)
    {
        do_log(LOG_ERROR, "Connection failed! no destiny to fd [%d]", vfd);
        goto scc_error;
    }

    ret = connect(vfd, target->ai_addr, target->ai_addrlen);

    if (ret < 0 && ignore_error(errno) == 0)
    {
        do_log(LOG_DEBUG, "Connect from fd [%d] failed!", vfd);
        goto scc_error;
    }

    if (connection_in_loop(target, vfd))
    {
        do_log(LOG_DEBUG, "Connection to myself is not allowed!");
        goto scc_error;
    }

    if (server_notify(rena, vfd, EPOLLOUT) < 0)
    {
        do_log(LOG_DEBUG, "server notify failed to fd [%d]!", vfd);
        goto scc_error;
    } else {
        do_log(LOG_DEBUG, "fd:%d waiting to write!", vfd);
    }

    return 0;

scc_error:
    proc_close(vfd);
    clients_set_fd(peer, -1);
    return -3;
}

int server_try_client_connect(struct rena *rena, void *peer)
{
    struct addrinfo *target = clients_get_userdata(peer);
    int vfd = clients_get_fd(peer);
    int ret=-3;

    while (ret != 0 && target != NULL)
    {
        ret = server_client_connect(rena, peer);

        if (ret == 0)
            break;

        target = target->ai_next;
        if (target == NULL)
            break;
        clients_set_userdata(peer, target);

        vfd = server_socket_for_client(rena, target->ai_addr);
        if (vfd < 0)
            break;
        clients_set_fd(peer, vfd);
    }

    do_log(LOG_DEBUG, "Returning code [%d] to socket [%d]",
           ret, vfd);
    return ret;
}

int server_client_set_ssl_data(struct rena *rena, void *data, int fd)
{
    SSL *s = data;

    if (!SSL_set_fd(s, fd)) {
        do_log(LOG_ERROR, "Error setting ssl data");
        return -1;
    }
    SSL_set_connect_state(s);
    return 0;
}

void server_close_client(int fd, void *is_ssl, int ret_rw)
{

    SSL *ssl = (void *) is_ssl;
    if (ssl && ret_rw != -2)
    {
        int ret = 0;
        for (int i=0; ret <= 0 && i < 5; i++)
        {
            ret = SSL_shutdown(ssl);
            do_log(LOG_DEBUG,
                    "fd:%d ssl shutdown returning [%d]",
                    fd, ret);
            if (ret == 0)
            {
                int pending = SSL_pending(ssl);
                if (pending > 0)
                {
                    char buf[MAX_STR*4];
                    int rd = SSL_read(ssl, buf, (int)sizeof(buf));
                    do_log(LOG_DEBUG, "fd:%d draining %d bytes...",
                            fd, rd);
                }

                ERR_clear_error();
            }
        }

        if (ret < 0)
        {
            do_log(LOG_ERROR,
                    "fd:%d ssl shutdown failed with [%d]",
                    fd, ssl_error(fd, ssl, ret, errno));
        }
    }

    fsync(fd);
    close(fd);
}

void server_free_ssl_client(void **is_ssl)
{
    if (*is_ssl == NULL)
        return ;

    SSL *ssl = (SSL *) *is_ssl;
    SSL_shutdown(ssl);
    SSL_free(ssl);
    *is_ssl = NULL;
}

void server_verify_task_number_change(struct rena *rena, double ratio)
{
    task_manager_can_notify_change_of_tasks(rena, ratio);
}
