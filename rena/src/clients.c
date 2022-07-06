#include "global.h"
#include "access.h"
#include "http.h"
#include "proc.h"
#include "server.h"

#include <openssl/ssl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

struct client_info
{
    char ip[INET6_ADDRSTRLEN];
    int tcp_connected : 1;
    int ssl_connected : 1;
    int handshake_done : 1;
    int working : 1;
    int invalid : 1;
    int fd;
    int desired_state;
    int last_desired_state;
    char want_ssl;
    time_t arrived_timestamp;
    time_t modified_timestamp;
    SSL *ssl;
    void *protocol;
    pthread_mutex_t protocol_lock;
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

static int client_protocol_lock(pthread_mutex_t *mutex, int side)
{
    text_t buf;
    if (side > 0)
    {
        if (pthread_mutex_lock(mutex) < 0) {
            proc_errno_message(&buf);
            do_log(LOG_ERROR, "pthread_mutex_lock fail: %s", buf.text);
            return -1;
        }
        return 0;
    } else if (side < 0)
    {
        if (pthread_mutex_unlock(mutex) < 0) {
            proc_errno_message(&buf);
            do_log(LOG_ERROR, "pthread_mutex_unlock fail: %s", buf.text);
            return -1;
        }
        return 0;
    }
    return -2;
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

int clients_protocol_lock(client_position_t *p, int get_change)
{
    struct client_info *ci = NULL;
    if (!p || !p->info) return 0;
    ci = (struct client_info *) p->info;
    if (get_change > 0 && clients_change_lock(1) != 0)
      {
        do_log(LOG_ERROR, "error getting change lock");
        return -3;
      }
    return client_protocol_lock(&ci->protocol_lock, 1);
}

int clients_protocol_unlock(client_position_t *p, int get_change)
{
    struct client_info *ci = NULL;
    if (!p || !p->info) return 0;
    ci = (struct client_info *) p->info;
    int ret = client_protocol_lock(&ci->protocol_lock, -1);
    if (get_change > 0 && clients_change_lock(-1) != 0)
      {
        do_log(LOG_ERROR, "error releasing change lock");
        return -3;
      }
    return ret;
}

static void client_info_destroy(struct client_info **ci, int delete_all)
{
    int ofd = -1;
    if (ci == NULL || *ci == NULL)
        return ;

    server_free_ssl_client((void **) &ci[0]->ssl);

    if (ci[0]->fd >= 0)
    {
        ofd = ci[0]->fd;
        fsync(ofd);
        close(ofd);
        ci[0]->fd = -1;
    }

    do_log(LOG_DEBUG, "sockets [%d] closed, delete data=%d", ofd, delete_all);
    if (delete_all != 1)
    {
        return ;
    }

    if (client_protocol_lock(&ci[0]->protocol_lock, 1) != 0)
    {
        do_log(LOG_ERROR,
               "error to lock to destroy client protocol!");
    }
    http_destroy(ci[0]->protocol);
    if (client_protocol_lock(&ci[0]->protocol_lock, -1) != 0)
    {
        do_log(LOG_ERROR,
               "error to unlock to destroy client protocol!");
    }
    if (pthread_mutex_destroy(&ci[0]->protocol_lock) != 0)
    {
        do_log(LOG_ERROR,
               "error to destroy protocol lock!");
    }
    free(*ci);
    *ci = NULL;
}

void clients_destroy(struct clients **clients)
{
    if (!clients || !*clients) return ;
    if (clients_change_lock(1) != 0)
    {
        do_log(LOG_ERROR, "error to get lock to destroy clients!");
        return ;
    }

    struct circle_client_info *cci = clients[0]->cci;
    if (cci != NULL)
    {
        cci->prev->next = NULL;
    }

    while (cci != NULL)
    {
        struct circle_client_info *aux = cci;
        cci = cci->next;
        if(aux->requester && aux->requester->userdata)
        {
            freeaddrinfo(aux->requester->userdata);
        }
        client_info_destroy(&aux->requester, 1);
        client_info_destroy(&aux->victim, 1);
        free(aux);
    }

    free(*clients);
    *clients = NULL;

    if (clients_change_lock(-1) != 0)
    {
        do_log(LOG_ERROR, "error to get lock to destroy clients!");
        return ;
    }
}

static void getpeer(struct client_info *ci)
{
    struct sockaddr_in6 clientaddr = { 0, };
    unsigned int addrlen = sizeof(clientaddr);
    if (!ci || ci->fd < 0)
        return ;
    if (getpeername(ci->fd, (struct sockaddr *)&clientaddr, &addrlen))
        return ;
    if(!inet_ntop(AF_INET6, &clientaddr.sin6_addr, ci->ip, sizeof(ci->ip)))
    {
        *ci->ip = 0;
    }
}

static int clients_add_ci(struct client_info **ci, int fd)
{
    if (ci == NULL)
        return -1;

    *ci = calloc(1, sizeof(struct client_info));
    (*ci)->arrived_timestamp = time(NULL);
    (*ci)->modified_timestamp = (*ci)->arrived_timestamp;
    (*ci)->fd = fd;
    (*ci)->desired_state = (*ci)->last_desired_state = 0;

    getpeer(*ci);
    if (pthread_mutex_init(&(*ci)->protocol_lock, NULL) != 0)
    {
        do_log(LOG_ERROR,
               "failed to initialize mutex for protocol on fd[%d]",
               fd);
        free(*ci);
        *ci = NULL;
        return -1;
    }
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

static int clients_search_pointer_locked(struct clients *cs,
                   struct circle_client_info *to_search)
{
    struct circle_client_info *cci = NULL;
    struct circle_client_info *aux = NULL;

    cci = aux = cs->cci;
    if (aux != NULL && to_search != NULL)
    {
        do
        {
            if (aux == to_search)
            {
                return 0;
            }
            aux = aux->next;
        } while (aux != cci);
    }

    return 1;
}

static void clients_del2_destroy_client_by_type(client_position_t *p,
                                                struct circle_client_info *cci)
{
    if (p->type == REQUESTER_TYPE)
    {
        int logged = log_access(p);
        if(cci->requester)
        {
            if(cci->requester->userdata)
                freeaddrinfo(cci->requester->userdata);
            cci->requester->invalid = 0; // to allow try again via fake task
        }
        do_log(LOG_DEBUG,
                "requester done fd:%d ip[%s] access=%d",
                cci->requester->fd, cci->requester->ip, logged);
        client_info_destroy(&cci->requester, 1);
    }

    if (p->type == VICTIM_TYPE || cci->requester == NULL)
    {
        int flag = (cci->requester == NULL) ? 1 : 0;
        if (cci->victim != NULL)
        {
            if (cci->victim->fd < 0
                    && http_sent_done(cci->victim->protocol) != 0)
            {
                flag = 1;
            }
            do_log(LOG_DEBUG,
                    "victim done fd:%d ip[%s] flag=%d",
                    cci->victim->fd, cci->victim->ip, flag);
            cci->victim->invalid = 0; // to allow try again via fake task
            client_info_destroy(&cci->victim, flag);
        }
    }
}

static int clients_del2(struct clients *cs, client_position_t *p)
{
    struct circle_client_info *cci
        = (struct circle_client_info *) p->pos;
    int erased_before = clients_search_pointer_locked(cs, cci);
    int ret = 1;
    if (erased_before)
    {
        return ret;
    }

    clients_del2_destroy_client_by_type(p, cci);

    if (cci->requester == NULL && cci->victim == NULL)
    {
        cci->next->prev = cci->prev;
        cci->prev->next = cci->next;
        if (cs->cci == cci)
        {
            cs->cci = (cci != cci->next) ? cci->next : NULL;
        }
        do_log(LOG_DEBUG, "both client done!");
        free(cci);
        cs->qty -= 1;
        p->info = NULL;
        p->pos = NULL;
        p->type = INVALID_TYPE;
    }

    ret = 0;
    if (cs->qty < 0)
    {
        do_log(LOG_ERROR,
                "wrong call order of add/del clients: %d", cs->qty);
        abort();
    }

    return ret;
}

int clients_del(struct clients *cs, client_position_t *p)
{
    int ret = 1;
    if (!p|| !p->info || !p->pos || p->type == INVALID_TYPE)
    {
        return -1;
    }

    if (clients_change_lock(1) != 0)
        return -1;

    ret = clients_del2(cs, p);

    if (clients_change_lock(-1) != 0)
    {
        abort();
    }

    return ret;
}

int clients_search(struct clients *cs, int fd, client_position_t *out)
{
    struct circle_client_info *cci = NULL;
    struct circle_client_info *aux = NULL;
    int ret = 1;

    if (clients_change_lock(1) != 0)
        return -1;

    cci = aux = cs->cci;
    if (aux != NULL)
    {
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
                if (aux->requester && aux->requester->fd >= 0)
                {
                    out->type = VICTIM_TYPE;
                    out->info = (const struct client_info *) aux->victim;
                    out->pos = (const void *) aux;
                    ret = 0;
                    break;
                }
            }
            aux = aux->next;
        } while (aux != cci);
    }

    if (clients_change_lock(-1) != 0)
    {
        abort();
    }
    return ret;
}

static int clients_alive_notify(struct rena *rena, struct client_info *ci)
{
    if (ci->fd >= 0)
    {
        if (ci->desired_state != ci->last_desired_state)
        {
            int res = server_notify(rena, ci->fd, ci->desired_state);
            if (res < 0)
            {
                return -1;
            }
            ci->last_desired_state = ci->desired_state;
        }
    }

    return 0;
}

static int clients_alive_check_modified(struct rena *rena,
                                        struct client_info *ci,
                                        time_t now, int secs, int *ret)
{
    int delta = (now - ci->modified_timestamp);

    if (delta >= secs)
    {
        if (delta > *ret)
            *ret = delta;
        return delta;
    }

    if (clients_alive_notify(rena, ci))
        return -2;

    return 0;
}

static int clients_alive_circle_client_info(struct rena *rena,
                                            struct circle_client_info *cci,
                                            int secs, int req_limit)
{
#define SEND_INVALID_TASK do {           \
    if (!ci->invalid) server_tm_push(rena, ci->fd, 0); \
    ci->invalid = 1; } while (0)

    if (cci == NULL)
        return 0;

    time_t now = time(NULL);
    struct client_info *ci = cci->requester;
    int ret = 1;

    if (ci)
    {
        time_t delta = (now - ci->arrived_timestamp);
        if (delta >= req_limit)
        {
            ret = delta;
            SEND_INVALID_TASK;

        } else {
            if (clients_alive_check_modified(rena, ci, now, secs, &ret) < 0)
                SEND_INVALID_TASK;
        }
    }

    ci = cci->victim;
    if (ci && proc_valid_fd(ci->fd))
    {
        int temp = clients_alive_check_modified(rena, ci, now, secs, &ret);
        if (temp)
        {
            SEND_INVALID_TASK;
            if (temp > ret) ret = temp;
        }
    }

    return ret;
#undef SEND_INVALID_TASK
}

void clients_alive(struct rena *rena, struct clients *c)
{
    if (c == NULL)
        return ;

    if (clients_change_lock(1) != 0)
    {
        do_log(LOG_ERROR, "error to get lock to destroy clients!");
        return ;
    }

    struct circle_client_info *cci_start = c->cci;
    struct circle_client_info *cci = cci_start;
    int reported = 0;
    int reported_time = 0;
    int pos = 0;
    int secs = 10;
    int req_limit = 120;
    int temp = clients_alive_circle_client_info(rena, cci, secs, req_limit);

    if (temp == 0)
    {
        if (clients_change_lock(-1) != 0)
        {
            do_log(LOG_ERROR, "error to get lock to destroy clients!");
            return ;
        }
        return ;
    }

    pos++;
    if (temp > 1)
    {
        reported++;
        reported_time = temp;
    }

    for (cci = cci->next; cci != cci_start; cci = cci->next)
    {
        pos++;
        temp = clients_alive_circle_client_info(rena, cci, secs, req_limit);
        if (temp > 1)
        {
            reported++;
            if (temp > reported_time)
                reported_time = temp;
        }
    }

    if (reported > 0)
    {
        double ratio = ((double)reported / (double) pos)
                     * (reported_time / secs);
        server_verify_task_number_change(rena, ratio);
    } else {
        server_verify_task_number_change(rena, 0.0);
    }

    if (clients_change_lock(-1) != 0)
    {
        do_log(LOG_ERROR, "error to get lock to destroy clients!");
        return ;
    }
}

void clients_set_tcp(client_position_t *p, int state)
{
    if (p == NULL)
    {
        return ;
    }

    struct client_info *ci = (struct client_info *) p->info;
    ci->tcp_connected=state;
    getpeer(ci);
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

void clients_set_desired_state(client_position_t *p, int state)
{
    if (p == NULL)
    {
        return ;
    }

    struct client_info *ci = (struct client_info *) p->info;
    ci->desired_state = state;
    // set zero to force a refresh
    ci->last_desired_state = 0;
}

void clients_set_want(client_position_t *p, char my_want, char ssl_want)
{
    struct client_info *pi = (struct client_info *) p->info;
    pi->want_ssl = (my_want << 2) + (ssl_want << 1) + 1;
    do_log(LOG_DEBUG, "client want ssl [%d] my [%s] ssl [%s]",
            pi->want_ssl,
            ((my_want != 0)?"READ":"WRITE"),
            ((ssl_want != 0)?"READ":"WRITE"));
}

void clients_clear_want(client_position_t *p)
{
    struct client_info *pi = (struct client_info *) p->info;
    if (pi->want_ssl != 0)
        do_log(LOG_DEBUG, "client clearing ssl want [%d]",
               pi->want_ssl);
    pi->want_ssl = 0;
}

int clients_get_want(client_position_t *p)
{
    struct client_info *pi = (struct client_info *) p->info;
    return pi->want_ssl;
}

int clients_get_desired_state(client_position_t *p)
{
    struct client_info *pi = (struct client_info *) p->info;
    return pi->desired_state;
}

int clients_add_peer(client_position_t *p, int fd)
{
    if (p == NULL)
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
    if (p == NULL || p->type == INVALID_TYPE || out == NULL)
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

const time_t *clients_get_arrived_timestamp(client_position_t *p)
{
    return &((struct client_info *) p->info)->arrived_timestamp;
}

const time_t *clients_get_timestamp(client_position_t *p)
{
    return &((struct client_info *) p->info)->modified_timestamp;
}

static void update_modified_timestamp(client_position_t *cp)
{
    struct client_info *ci = (struct client_info *) cp->info;
    if (!ci) return ;
    ci->modified_timestamp = time(NULL);
}

int client_do_read(struct rena *rena, client_position_t *c, int fd)
{
    clients_protocol_lock(c, 0);
    int ret = http_pull(rena, c, fd);
    if (ret != 0)
    {
        clients_protocol_unlock(c, 0);
        if (ret < 0) return -1;
        return ret;
    }
    ret = http_evaluate(rena, c);
    update_modified_timestamp(c);
    clients_protocol_unlock(c, 0);
    return ret;
}

int client_do_write(struct rena *rena, client_position_t *c, int fd)
{
    client_position_t peer_raw = {NULL, INVALID_TYPE, NULL};
    client_position_t *peer = &peer_raw;
    int ret = -1;

    clients_protocol_lock(c, 0);
    clients_get_peer(c, &peer_raw);
    clients_protocol_lock(peer, 0);

    ret = http_push(rena, c, fd, peer);
    update_modified_timestamp(c);
    update_modified_timestamp(peer);

    clients_protocol_unlock(peer, 0);
    clients_protocol_unlock(c, 0);
    if (ret < 0) return -1;
    if (ret > 0) return ret;
    return 0;
}


static int client_log_bytes_received_from_victim(client_log_format_t *lf)
{
    struct circle_client_info *cci
                = (struct circle_client_info *) lf->client->pos;
    if (!cci->victim) return -1;
    void *cprot = cci->victim->protocol;
    return http_bytes_sent(cprot, lf->out, lf->out_sz);
}

static int client_log_status_code_received_from_victim(client_log_format_t *lf)
{
    struct circle_client_info *cci
                = (struct circle_client_info *) lf->client->pos;
    if (!cci->victim) return -1;
    void *cprot = cci->victim->protocol;
    return http_status(cprot, lf->out, lf->out_sz);
}

static int client_log_ip_from_requester(client_log_format_t *lf)
{
    struct circle_client_info *cci
                = (struct circle_client_info *) lf->client->pos;
    struct client_info *ci = cci->requester;

    int length = strlen(ci->ip);
    if (length < 1 || length > lf->out_sz)
        return -1;

    memcpy(lf->out, ci->ip, length);
    return length;
}

static int client_log_request_received_from_requester(client_log_format_t *lf)
{
    struct circle_client_info *cci
                = (struct circle_client_info *) lf->client->pos;
    void *cprot = cci->requester->protocol;
    return http_request_line(cprot, lf->out, lf->out_sz, cci->requester->ssl);
}

static int client_log_header_from_both(client_log_format_t *lf)
{
    if (lf->attrib_len <= 0 || lf->attrib == 0)
        return -1;

    struct circle_client_info *cci
                = (struct circle_client_info *) lf->client->pos;
    void *cprot = cci->requester->protocol;
    int rh = http_find_header(cprot, lf->attrib, lf->attrib_len);

    if (rh < 0 && cci->victim)
    {
        cprot = cci->victim->protocol;
        rh = http_find_header(cprot, lf->attrib, lf->attrib_len);
    }

    if (rh < 0)
        return -2;

    int ret = http_header_value(cprot, lf->out, lf->out_sz, rh);
    return (ret < 0) ? -2 : ret;
}


int client_do_log_format(client_log_format_t *lf)
{
    typedef int (*fnc_handler_t)(client_log_format_t *);
    fnc_handler_t handlers[LOG_FORMAT_LAST] = {
            client_log_bytes_received_from_victim,
            client_log_status_code_received_from_victim,
            client_log_ip_from_requester,
            client_log_request_received_from_requester,
            client_log_header_from_both
        };
    int ret = -1;
    if (lf && lf->type < LOG_FORMAT_LAST && lf->type >= 0)
    {
    //LATER TODO -- review if need lock here! maybe move to inside?
    //LATER TODO -- review if need lock here the peer!
    //clients_protocol_lock(c, 0);
    ret = handlers[lf->type](lf);
    //clients_protocol_unlock(c, 0);
    }
    return ret;
}
