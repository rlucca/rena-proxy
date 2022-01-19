
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
    int s = proc_receive_signal(task->fd);
    if (s < 0)
    {
        return EPOLLIN;
    }

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

        if (err || server_notify(rena, EPOLL_CTL_ADD, fd, EPOLLIN))
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
    int tcpok = clients_get_tcp(c);
    int sslok = clients_get_ssl_state(c);

    if (!sslok && ssl)
    {
        int r = server_handshake_client(task->fd, ssl);
        if (r != 0)
        {
            if (r < 0)
                return -1;
            return (r == TT_READ)?EPOLLIN:EPOLLOUT;
        } else {
            clients_set_ssl_state(c, 1);
        }
    }

    if (tcpok == 0)
        clients_set_tcp(c, 1);

    return 0;
}

static int handle_client_read(struct rena *rena, task_t *task,
                              client_position_t *c)
{
    int tcpok = clients_get_tcp(c);
    void *ssl = clients_get_ssl(c);

    if (tcpok == 0)
    {
        int r = 0;
        if ((r = handle_handshake(task, c, ssl)) != 0)
            return r;
        do_log(LOG_DEBUG, "done handshake!");
        return EPOLLIN;
    }

    return client_do_read(rena, c, task->fd);
}

static int handle_client_write(struct rena *rena, task_t *task,
                               client_position_t *c)
{
    int tcpok = clients_get_tcp(c);
    void *ssl = clients_get_ssl(c);

    if (tcpok == 0)
    {
        int r = 0;
        if ((r = handle_handshake(task, c, ssl)) != 0)
            return r;
        do_log(LOG_DEBUG, "done handshake!");
        return EPOLLIN;
    }

    return client_do_write(rena, c, task->fd);
}

static void task_handling(struct rena *rena, task_t *task)
{
    client_position_t cp = {NULL, INVALID_TYPE, NULL};
    int (*fnc_read)(struct rena *, task_t *, client_position_t *);
    int (*fnc_write)(struct rena *, task_t *, client_position_t *);
    int mod_fd = 0;
    int error = 1;

    if (task == NULL)
    {
        return ;
    }

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
            abort();
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

    if (cp.type != INVALID_TYPE)
    {
        clients_set_working(&cp, 0);
    }

    if (error > 0)
    {
        do_log(LOG_ERROR,
               "task [%d] fd [%d] do invalid!",
               task->type, task->fd);
    }

    if (mod_fd <= 0)
    {
        if (cp.type != INVALID_TYPE)
        {
            clients_del(rena->clients, &cp);
        }
    } else {
        if (server_notify(rena, EPOLL_CTL_MOD, task->fd, mod_fd) < 0)
        {
            abort();
        }
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
