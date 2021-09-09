#include "globals.h"
#include "http.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t delete_mutex = PTHREAD_MUTEX_INITIALIZER;

// Aqui nao interessa se eh HTTP / HTTPS, entao fica 0...
DECLARE_ENQUEUE_ACTION(read_socket_f, ACTION_READ, 0)
DECLARE_ENQUEUE_ACTION(write_socket_f, ACTION_WRITE, 0)

static int get_ip_str(int sockfd, char *s, size_t maxlen)
{
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);

    if (getpeername(sockfd, &addr, &addrlen) != 0)
    {
        fprintf(stderr, "getpeername failed (%d)", errno);
        strncpy(s, "getpeername failed", maxlen);
        return -1;
    }

    switch(addr.sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)&addr)->sin_addr),
                    s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&addr)->sin6_addr),
                    s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return -1;
    }

    return 0;
}

fa_request_t *fa_request_create(int client, SSL *ssl, int side)
{
    fa_request_t *req = calloc(1, sizeof(fa_request_t));
    req->fd = client;
    req->related_fd = -1;
    ev_io_init(&req->read_watcher, read_socket_f, client, EV_READ);
    ev_io_init(&req->write_watcher, write_socket_f, client, EV_WRITE);
    req->ctx = ssl;
    req->side = side;

    if (req->side > 0) {
        get_ip_str(client, req->user_ip, sizeof(req->user_ip));
        req->user_ip[MAX_STR - 1] = '\0';
    }

    req->creation_at = time(NULL);
    http_payload_init(req);

    pthread_mutex_init(&req->mutex, NULL);
    return req;
}

int fa_request_curr_state(fa_request_t *fa)
{
	pthread_mutex_lock(&(fa->mutex));
	int ret = fa->curr_state;
	pthread_mutex_unlock(&(fa->mutex));
    return ret;
}

void fa_request_curr_state_set(fa_request_t *fa, int cs)
{
	pthread_mutex_lock(&(fa->mutex));
	fa->curr_state = cs;
	pthread_mutex_unlock(&(fa->mutex));
}

size_t fa_request_remain_capacity(fa_request_t *fa)
{
    size_t ret = sizeof(fa->payload);

	pthread_mutex_lock(&(fa->mutex));
    ret -= fa->payload_size;
	pthread_mutex_unlock(&(fa->mutex));

    return ret;
}

size_t fa_request_size(fa_request_t *fa)
{
    size_t ret;

	pthread_mutex_lock(&(fa->mutex));
    ret = fa->payload_size;
	pthread_mutex_unlock(&(fa->mutex));

    return ret;
}

int fa_request_payload_transform(fa_request_t *fa,
                                 const char *buffer,
                                 size_t buffer_sz)
{
    int ret = 0;

    // Nao seguramos o(s) lock(s) aqui porque
    // as funcoes chamadas ja se encarregam de
    // realizar essas atitudes!

    // -1, erro
    //  0, ok e expandido
    //  1, ok, mas so tem coisa segurada
    ret = http_payload_loader(fa, buffer, buffer_sz);

    return ret;
}

fa_request_t *fa_request_push_holding(fa_request_t *fa)
{
    fa_request_t *ret = NULL;
	pthread_mutex_lock(&(fa->mutex));

    ret = http_payload_holding_loader(fa);

	pthread_mutex_unlock(&(fa->mutex));

    return ret;
}

int fa_request_payload_put(fa_request_t *fa,
                           const char *buffer,
                           size_t buffer_sz)
{
    size_t u;

	pthread_mutex_lock(&(fa->mutex));
    if (fa->payload_size == sizeof(fa->payload))
    {
        DEBUG("sem capacidade de receber");
        pthread_mutex_unlock(&(fa->mutex));
        return -1;
    }

    for (u = 0;
         u < buffer_sz && fa->payload_size < sizeof(fa->payload);
         u++, fa->payload_size++)
    {
        fa->payload[fa->payload_w] = buffer[u];
        fa->payload_w = (fa->payload_w + 1) % sizeof(fa->payload);
    }

	pthread_mutex_unlock(&(fa->mutex));
    return u;
}

void fa_request_payload_consume(fa_request_t *fa,
                                size_t buffer_sz)
{
    size_t max;
    size_t to_fill = sizeof(fa->payload);

	pthread_mutex_lock(&(fa->mutex));

    if (fa->payload_size == 0)
    {
        DEBUG("payload vazio");
        pthread_mutex_unlock(&(fa->mutex));
        return ;
    }

    if (buffer_sz < fa->payload_size)
        max = buffer_sz;
    else
        max = fa->payload_size;

    fa->payload_r = (fa->payload_r + max) % sizeof(fa->payload);
    fa->payload_size = fa->payload_size - max;
    to_fill -= fa->payload_size;

	pthread_mutex_unlock(&(fa->mutex));

    http_payload_fill_output(fa, to_fill); // lock again
    return ;
}

int fa_request_payload_copy(fa_request_t *fa,
                            char *buffer,
                            size_t buffer_sz)
{
    size_t u = 0;
    size_t min;

	pthread_mutex_lock(&(fa->mutex));

    if (fa->payload_size == 0)
    {
        DEBUG("payload vazio");
        pthread_mutex_unlock(&(fa->mutex));
        return -1;
    }

    if (buffer_sz < fa->payload_size)
        min = buffer_sz;
    else
        min = fa->payload_size;

    for (u = 0; u < min; u++)
    {
        size_t read_at = (fa->payload_r + u) % sizeof(fa->payload);
        buffer[u] = fa->payload[read_at];
    }

	pthread_mutex_unlock(&(fa->mutex));
    return min;
}

int fa_request_delete(fa_request_t *fa)
{
    fa_request_t *related = NULL;

    // para a delecao temos um mutex auxiliar para
    // enfileirar o acesso! Ja que podemos ter threads
    // diferentes em diferentes pontos...
	pthread_mutex_lock(&delete_mutex);
	pthread_mutex_lock(&(fa->mutex));

    if (fa->curr_state != 0)
    {
        DEBUG("Ignorei por estar no estado %d", fa->curr_state);
        pthread_mutex_unlock(&(fa->mutex));
        pthread_mutex_unlock(&delete_mutex);
        return -1;
    }

    ev_io_stop(events.main_loop, &fa->read_watcher);
    ev_io_stop(events.main_loop, &fa->write_watcher);
    ev_async_send(events.main_loop, &events.wake_up);

    if (!(fa->related_fd < 0))
    {
        related = requests[fa->related_fd];
        if (related)
        {
            pthread_mutex_lock(&(related->mutex));
            related->related_fd = -1;
            pthread_mutex_unlock(&(related->mutex));
        }
    }
    if (fa->ctx)
    {
        SSL_free(fa->ctx);
        fa->ctx = NULL;
    }

    fa_request_close_socket(fa->fd);
    requests[fa->fd] = NULL;
    http_payload_release(fa);

	pthread_mutex_unlock(&(fa->mutex));
    pthread_mutex_destroy(&(fa->mutex));
    free(fa);
	pthread_mutex_unlock(&delete_mutex);
    return 0;
}

void fa_request_close_socket(int fd)
{
    // deveria serchamado somente segurando o
    // lock interno para enfileirar, mas ha um
    // caso que nao precisa entao nao garantiremos
    shutdown(fd, SHUT_RDWR);
    close(fd);
}
