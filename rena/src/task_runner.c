
#include "global.h"
#include "task_manager.h"
#include "task_runner.h"
#include "server.h"
#include "proc.h"
#include "clients.h"

#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void task_handling(struct rena *rena, task_t *task);

static int has_io(int io)
{
    static int ret = -1;
    if (io != 0)
    {
        ret = io;
    }

    return ret;
}

void *task_runner(void *arg)
{
    struct rena *rena = (struct rena *) arg;

    while (rena->forced_exit == 0)
    {
        int do_dispatch = 0;
        if (has_io(0) < 0)
        {
            THREAD_CRITICAL_BEGIN(lock)
            if (has_io(0) < 0)
            {
                has_io(1);
                do_dispatch = 1;
            }
            THREAD_CRITICAL_END(lock)
        }

        if (do_dispatch != 0)
        {
            task_manager_set_working(rena, 1);
            server_dispatch(rena);
            task_manager_set_working(rena, 0);

            THREAD_CRITICAL_BEGIN(lock)
            if (has_io(0) != 0)
            {
                has_io(-1);
            }
            THREAD_CRITICAL_END(lock)
        } else {
            task_t *task = NULL;
            task_manager_set_working(rena, 1);
            task = task_manager_task_consume(rena);
            task_handling(rena, task);
            task_manager_set_working(rena, 0);
            task_manager_task_free(&task);
        }
    }

    task_manager_forced_exit(rena);
    return NULL;
}

static int handle_read_signal(struct rena *rena, task_t *task,
                              client_position_t *c)
{
    int s = -1;
    while ((s = proc_receive_signal(task->fd)) >= 0)
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
    }

    return EPOLLIN;
}

static int handle_accept(struct rena *rena, int svr, void **ssl)
{
    int fd = -2;
    while ((fd = server_receive_client(rena, svr, ssl)) >= 0)
    {
        int err = 0;
        if (clients_add(rena->clients, REQUESTER_TYPE, fd))
        {
            err = 1;
        }

        if (!err && ssl && *ssl)
        {
            client_position_t out;
            if (clients_search(rena->clients, fd, &out))
                err = 2;
            else
                clients_set_ssl(&out, *ssl);
        }

        if (err || server_notify(rena, EPOLL_CTL_ADD, fd, EPOLLOUT))
        {
            err = 3;
        }

        if (err != 0) // something wrong!
        {
            do_log(LOG_ERROR, "dropping client [%d]", fd);
            proc_close(fd);
            break;
        }
    }

    return EPOLLIN;
}

static int handle_accept_http(struct rena *rena, task_t *task,
                              client_position_t *c)
{
    return handle_accept(rena, task->fd, NULL);
}

static int handle_accept_https(struct rena *rena, task_t *task,
                              client_position_t *c)
{
    void *ssl;
    return handle_accept(rena, task->fd, &ssl);
}

static int handle_handshake(task_t *task, client_position_t *c, void *ssl)
{
    int sslok = clients_get_ssl_state(c);

    if (!sslok && ssl)
    {
        int r = server_handshake_client(task->fd, ssl);
        if (r != 0)
        {
            if (r < 0)
            {
                do_log(LOG_DEBUG, "handshake failed on fd:%d", task->fd);
                return -1;
            }

            do_log(LOG_DEBUG, "handshake need future event on fd:%d", task->fd);
            return (r == TT_READ)?EPOLLIN:EPOLLOUT;
        } else {
            clients_set_ssl_state(c, 1);
        }
    }

    clients_set_handshake(c, 1);
    do_log(LOG_DEBUG, "handshake complete on fd:%d", task->fd);
    return 0;
}

static int handle_client_read(struct rena *rena, task_t *task,
                              client_position_t *c)
{
    int tcpok = clients_get_tcp(c);
    void *ssl = clients_get_ssl(c);
    int hnd = clients_get_handshake(c);
    int r = -1;

    do_log(LOG_DEBUG, "fd:%d tcp [%d] handshake [%d] is_ssl [%p]",
           task->fd, tcpok, hnd, ssl);

    if (tcpok == 0)
        return EPOLLOUT;

    if (hnd == 0)
    {
        int r = 0;
        if ((r = handle_handshake(task, c, ssl)) != 0)
            return r;
        return (c->type == VICTIM_TYPE) ? EPOLLOUT : EPOLLIN;
    }

    r = client_do_read(rena, c, task->fd);
    if (r == TT_READ) return EPOLLIN;
    if (r == TT_WRITE) return EPOLLOUT;
    return r;
}

static int handle_client_write(struct rena *rena, task_t *task,
                               client_position_t *c)
{
    int tcpok = clients_get_tcp(c);
    void *ssl = clients_get_ssl(c);
    int hnd = clients_get_handshake(c);
    int r = -1;

    do_log(LOG_DEBUG, "fd:%d tcp [%d] handshake [%d] is_ssl [%p]",
           task->fd, tcpok, hnd, ssl);

    if (tcpok == 0)
    {
        if (server_tcp_connection_done(task->fd))
        {
            if (c->type != VICTIM_TYPE
                    || server_try_client_connect(rena, c) != 0)
            {
                return -1;
            }

            return EPOLLOUT;
        }

        clients_set_tcp(c, 1);
        do_log(LOG_DEBUG, "fd:%d tcp ok!", task->fd);

        if (ssl != NULL && c->type == VICTIM_TYPE)
        {
            server_client_set_ssl_data(rena, ssl, task->fd);
        }
    }

    if (hnd == 0)
    {
        int r = 0;
        if ((r = handle_handshake(task, c, ssl)) != 0)
            return r;
        return (c->type == VICTIM_TYPE) ? EPOLLOUT : EPOLLIN;
    }

    r = client_do_write(rena, c, task->fd);
    if (r == TT_READ) return EPOLLIN;
    if (r == TT_WRITE) return EPOLLOUT;
    return r;
}

static void task_delete_client(struct rena *rena,
                               task_t *task,
                               client_position_t *c)
{
    client_position_t p = {NULL, INVALID_TYPE, NULL};

    clients_get_peer(c, &p);
    if (p.info != NULL)
        server_notify(rena, EPOLL_CTL_MOD, clients_get_fd(&p), EPOLLOUT);
    if (!clients_del(rena->clients, c))
        task_manager_task_drop_fd(rena->tm, task->fd);
}

static void task_handling(struct rena *rena, task_t *task)
{
    client_position_t cp = {NULL, INVALID_TYPE, NULL};
    int (*fnc_read)(struct rena *, task_t *, client_position_t *);
    int (*fnc_write)(struct rena *, task_t *, client_position_t *);
    int mod_fd = 0;
    int error = 1;

    // task->fd == -1 is a logical deletion done previously,
    // so it is silent ignored
    if (task == NULL || task->fd < 0)
    {
        return ;
    }

    do_log(LOG_DEBUG, "Task %s from fd %d",
           ((task->type) & 0x01) ? "READ" : "WRITE",
           task->fd);

    fnc_read = NULL;
    fnc_write = NULL;
    if (task->type < TT_READ)
    {
        // nothing to set!
    } else if (task->type < TT_NORMAL_READ)
    {
        int cret = clients_search(rena->clients, task->fd, &cp);
        if (cret == 0)
        {
            fnc_read = handle_client_read;
            fnc_write = handle_client_write;
        }
    } else if (task->type < TT_SECURE_READ)
    {
        fnc_read = handle_accept_http;
        fnc_write = NULL;
    } else if (task->type < TT_SIGNAL_READ)
    {
        fnc_read = handle_accept_https;
        fnc_write = NULL;
    } else {
        fnc_read = handle_read_signal;
        fnc_write = NULL;
    }

    if (cp.type != INVALID_TYPE)
    {
        if(clients_get_working(&cp)!=0)
        {
            do_log(LOG_ERROR,
                   "client is in a working state. Should not be here");
            task_manager_task_push(rena, task->fd, task->type);
            return ;
        }

        clients_set_working(&cp, 1);
    }

    if ((task->type & 1) == 1)
    {
        do_log(LOG_DEBUG,
               "task [%d] fd [%d] do read!",
               task->type, task->fd);
        if (fnc_read)
        {
            mod_fd = fnc_read(rena, task, &cp);
            error = 0;
        }
    } else if (task->type > 0)
    {
        do_log(LOG_DEBUG,
               "task [%d] fd [%d] do write!",
               task->type, task->fd);
        if (fnc_write)
        {
            mod_fd = fnc_write(rena, task, &cp);
            error = 0;
        }
    }

    if (error > 0)
    {
        do_log(LOG_ERROR,
               "task [%d] fd [%d] client not found! (client disconnected?)",
               task->type, task->fd);
    }

    if (mod_fd <= 0 || !proc_valid_fd(task->fd))
    {
        task_delete_client(rena, task, &cp);
        error = 1;
    } else {
        int rn = 0;
        do_log(LOG_DEBUG, "modifying event notifier on fd %d to %d",
                task->fd, mod_fd);
        rn = server_notify(rena, EPOLL_CTL_MOD, task->fd, mod_fd);
        if (rn == -1)
        {
            do_log(LOG_ERROR, "failed to modified event notifier on fd [%d]",
                    task->fd);
            task_delete_client(rena, task, &cp);
            error = 1;
        } else {
            if (rn < 0)
            {
                rn = server_notify(rena, EPOLL_CTL_ADD, task->fd, mod_fd);
                if (rn < 0)
                {
                    do_log(LOG_ERROR,
                           "failed to add event notifier on fd [%d:%d] again",
                            task->fd, proc_valid_fd(task->fd));
                    task_delete_client(rena, task, &cp);
                    error = 1;
                }
            }
        }
    }

    if (error == 0 && cp.type == VICTIM_TYPE
        && clients_get_handshake(&cp) == 1
        && cp.info != NULL
        && clients_get_protocol(&cp) != NULL)
    {
        // lets notify the requester too!
        client_position_t peer_raw = {NULL, INVALID_TYPE, NULL};
        client_position_t *peer = &peer_raw;
        int pfd = -1;

        clients_get_peer(&cp, &peer_raw);
        if (peer_raw.info) pfd = clients_get_fd(peer);
        if (pfd < 0
            || server_notify(rena, EPOLL_CTL_MOD, pfd, EPOLLOUT) < 0)
        {
            do_log(LOG_DEBUG, "update notify fd [%d] peer of fd [%d] failed!",
                   pfd, task->fd);
        } else {
            do_log(LOG_DEBUG, "update notify fd [%d] peer of fd [%d] okay!",
                   pfd, task->fd);
        }
    }

    if (cp.type != INVALID_TYPE)
    {
        clients_set_working(&cp, 0);
    }
}

void task_runner_destroy(void *arg)
{
    struct rena *rena = (struct rena *) arg;

    // empty all queued data
    while (task_manager_task_queue_size(rena) > 0)
    {
        task_t *task = task_manager_task_consume(rena);
        task_manager_task_free(&task);
    }

    // delete all clients added
    clients_destroy(&rena->clients);
}
