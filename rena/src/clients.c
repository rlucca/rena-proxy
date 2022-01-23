#include "global.h"
#include "http.h"

#include <openssl/ssl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

struct client_info
{
    char ip[INET6_ADDRSTRLEN];
    int tcp_connected;
    int ssl_connected;
    int handshake_done;
    int working;
    int fd;
    SSL *ssl;
    void *protocol;
    void *userdata;
};

struct circle_client_info
{
    struct circle_client_info *next;
    struct circle_client_info *prev;
    struct client_info *requester;
    struct client_info *victim;
};

struct clients
{
    struct circle_client_info *cci;
    int qty;
};


struct clients *clients_init()
{
    return calloc(1, sizeof(struct clients));
}

int clients_quantity(struct clients *c)
{
    if (c == NULL)
        return 0;
    return c->qty;
}

static void client_info_destroy(struct client_info **ci)
{
    if (ci == NULL || *ci == NULL)
        return ;

    if (ci[0]->ssl) {
        SSL_shutdown(ci[0]->ssl);
        SSL_free(ci[0]->ssl);
        ci[0]->ssl = NULL;
    }
    close(ci[0]->fd);
    http_destroy(ci[0]->protocol);
    free(*ci);
    *ci = NULL;
}

void clients_destroy(struct clients **clients)
{
    if (!clients || !*clients) return ;
    struct circle_client_info *cci = clients[0]->cci;
    if (cci != NULL)
    {
        cci->prev->next = NULL;
    }

    while (cci != NULL)
    {
        struct circle_client_info *aux = cci;
        cci = cci->next;
        client_info_destroy(&aux->requester);
        client_info_destroy(&aux->victim);
        free(aux);
    }

    free(*clients);
    *clients = NULL;
}

static int clients_change_lock(int lock)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    if (lock > 0)
    {
        return pthread_mutex_lock(&mutex);
    }
    return pthread_mutex_unlock(&mutex);
}

static void getpeer(int fd, char *ip, size_t ip_len)
{
    struct sockaddr_in6 clientaddr;
    unsigned int addrlen = sizeof(clientaddr);
    if (*ip != 0 || fd < 0)
        return ;
    if (getpeername(fd, (struct sockaddr *)&clientaddr, &addrlen) == 0)
    {
        if(inet_ntop(AF_INET6, &clientaddr.sin6_addr, ip, ip_len) != 0)
        {
            *ip = 0;
        }
    }
}

static int clients_add_ci(struct client_info **ci, int fd)
{
    if (ci == NULL)
        return -1;

    *ci = calloc(1, sizeof(struct client_info));
    (*ci)->fd = fd;
    getpeer(fd, (*ci)->ip, sizeof((*ci)->ip));
    return 0;
}

int clients_add(struct clients *cs, client_type_e t, int fd)
{
    struct circle_client_info *cci = NULL;
    if (clients_change_lock(1) != 0)
        return -1;

    cci = calloc(1, sizeof(struct circle_client_info));
    if (t == REQUESTER_TYPE)
        clients_add_ci(&cci->requester, fd);
    else
        clients_add_ci(&cci->victim, fd);
    if (cs->cci != NULL)
    {
        cci->next = cs->cci;
        cci->prev = cs->cci->prev;
        cs->cci->prev->next = cci;
        cs->cci->prev = cci;
        cs->cci = cci;
    } else {
        cci->next = cci;
        cci->prev = cci;
        cs->cci = cci;
    }
    cs->qty += 1;
    if (clients_change_lock(-1) != 0)
    {
        abort();
    }
    return 0;
}

int clients_del(struct clients *cs, client_position_t *p)
{
    int ret = 1;
    if (!p|| !p->info || !p->pos
            || p->type == INVALID_TYPE)
    {
        return -1;
    }

    if (clients_change_lock(1) != 0)
        return -1;

    struct circle_client_info *cci
            = (struct circle_client_info *) p->pos;
    if (p->type == REQUESTER_TYPE)
    {
        client_info_destroy(&cci->requester);
    }
    else
    {
        client_info_destroy(&cci->victim);
    }

    if (cci->requester == NULL && cci->victim == NULL)
    {
        cci->next->prev = cci->prev;
        cci->prev->next = cci->next;
        if (cs->cci == cci)
        {
            cs->cci = (cci != cci->next) ? cci->next : NULL;
        }
        free(cci);
    }

    ret = 0;
    p->info = NULL;
    p->pos = NULL;
    p->type = INVALID_TYPE;
    cs->qty -= 1;
    if (cs->qty < 0)
    {
        do_log(LOG_ERROR,
               "wrong call order of add/del clients");
        abort();
    }

    if (clients_change_lock(-1) != 0)
    {
        abort();
    }

    return ret;
}

int clients_search(struct clients *cs, int fd,
                   client_position_t *out)
{
    struct circle_client_info *cci = NULL;
    struct circle_client_info *aux = NULL;
    int ret = 1;
    memset(out, 0, sizeof(*out));
    if (clients_change_lock(1) != 0)
        return -1;

    cci = aux = cs->cci;
    do
    {
        if (aux->requester && aux->requester->fd == fd)
        {
            out->type = REQUESTER_TYPE;
            out->info = (const struct client_info *) aux->requester;
            out->pos = (const void *) aux;
            ret = 0;
            break;
        }
        if (aux->victim && aux->victim->fd == fd)
        {
            out->type = VICTIM_TYPE;
            out->info = (const struct client_info *) aux->victim;
            out->pos = (const void *) aux;
            ret = 0;
            break;
        }
        aux = aux->next;
    } while (aux != cci);

    if (clients_change_lock(-1) != 0)
    {
        abort();
    }
    return ret;
}

void clients_set_tcp(client_position_t *p, int state)
{
    if (p == NULL)
    {
        return ;
    }

    struct client_info *ci = (struct client_info *) p->info;
    ci->tcp_connected=state;
    getpeer(ci->fd,
            ci->ip, sizeof(ci->ip));
}

void clients_set_working(client_position_t *p, int state)
{
    if (p == NULL)
    {
        return ;
    }

    ((struct client_info *) p->info)->working = state;
}

void clients_set_fd(client_position_t *p, int fd)
{
    if (p == NULL)
    {
        return ;
    }

    ((struct client_info *) p->info)->fd = fd;
}

void clients_set_ssl(client_position_t *p, void *ssl)
{
    if (p == NULL)
    {
        return ;
    }

    struct client_info *ci = (struct client_info *) p->info;
    ci->ssl = (SSL *) ssl;
}

void clients_set_ssl_state(client_position_t *p, int state)
{
    if (p == NULL)
    {
        return ;
    }

    struct client_info *ci = (struct client_info *) p->info;
    ci->ssl_connected = state;
}

void clients_set_protocol(client_position_t *p, void *s)
{
    struct client_info *pi = (struct client_info *) p->info;
    pi->protocol = s;
}

void clients_set_userdata(client_position_t *p, void *s)
{
    struct client_info *pi = (struct client_info *) p->info;
    pi->userdata = s;
}

void clients_set_handshake(client_position_t *p, int s)
{
    struct client_info *pi = (struct client_info *) p->info;
    pi->handshake_done = s;
}

int clients_add_peer(client_position_t *p, int fd)
{
    if (p == NULL || fd < 0)
    {
        return -1;
    }

    struct circle_client_info *cci = (void *) p->pos;
    if (cci == NULL || p->type == INVALID_TYPE)
    {
        return -2;
    }

    struct client_info **ci = NULL;
    if (p->type == REQUESTER_TYPE)
    {
        if (cci->victim != NULL)
        {
            return -3;
        }

        ci = &cci->victim;
    } else {
        if (cci->requester != NULL)
        {
            return -3;
        }

        ci = &cci->requester;
    }

    return clients_add_ci(ci, fd);
}

int clients_get_peer(client_position_t *p, client_position_t *out)
{
    if (p == NULL || out == NULL)
    {
        return -1;
    }

    out->type = (p->type == REQUESTER_TYPE)?VICTIM_TYPE:REQUESTER_TYPE;
    out->pos = p->pos;
    if (out->type == REQUESTER_TYPE)
        out->info = ((struct circle_client_info *) p->pos)->requester;
    else
        out->info = ((struct circle_client_info *) p->pos)->victim;
    return 0;
}

int clients_get_fd(client_position_t *p)
{
    return ((struct client_info *) p->info)->fd;
}

int clients_get_tcp(client_position_t *p)
{
    return ((struct client_info *) p->info)->tcp_connected;
}

int clients_get_working(client_position_t *p)
{
    return ((struct client_info *) p->info)->working;
}

int clients_get_ssl_state(client_position_t *p)
{
    return ((struct client_info *) p->info)->ssl_connected;
}

void *clients_get_ssl(client_position_t *p)
{
    return ((struct client_info *) p->info)->ssl;
}

const char *clients_get_ip(client_position_t *p)
{
    return ((struct client_info *) p->info)->ip;
}

void *clients_get_protocol(client_position_t *p)
{
    return ((struct client_info *) p->info)->protocol;
}

void *clients_get_userdata(client_position_t *p)
{
    return ((struct client_info *) p->info)->userdata;
}

int clients_get_handshake(client_position_t *p)
{
    return ((struct client_info *) p->info)->handshake_done;
}

int client_do_read(struct rena *rena, client_position_t *c, int fd)
{
    int ret = http_pull(rena, c, fd);
    if (ret < 0) return -1;
    if (ret > 0) return ret;
    return http_evaluate(rena, c);
}

int client_do_write(struct rena *rena, client_position_t *c, int fd)
{
    struct client_info *ci = (struct client_info *) c->info;
    //client_position_t peer;
    //int ret = 0;
    if (ci->fd != fd)
    {
        do_log(LOG_ERROR, "invalid client fd %d against %d (%s)",
               fd, ci->fd, ci->ip);
        return -1;
    }
    abort();
    // escritas sao sempre feitas a partir do protocolo da outra ponta!
    //clients_get_peer(ci->protocol, &peer);
    //ret = http_push_from_protocol(ci, fd, peer.info);
    //                      se fizer isso quebra o encapsulamento!
    //if (ret < 0) return -1;
    //if (ret > 0) return ret;
    //return http_evaluate_state(ci->protocol);
    return -1;
}
