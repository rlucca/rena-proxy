
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

static void task_handling(struct rena *rena, task_t *task)
{
    client_position_t cp = {NULL, INVALID_TYPE, NULL};
    int (*fnc_read)(struct rena *, task_t *, client_position_t *);
    int (*fnc_write)(struct rena *, task_t *, client_position_t *);
    int mod_fd = 0;

    do_log(LOG_DEBUG, "task [%p]: not implemented", task);

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
            fnc_read = NULL;
            fnc_write = NULL;
        }
    } else if (task->type < TT_SECURE_READ)
    {
        fnc_read = NULL;
        fnc_write = NULL;
    } else if (task->type < TT_SIGNAL_READ)
    {
        fnc_read = NULL;
        fnc_write = NULL;
    } else {
        fnc_read = NULL;
        fnc_write = NULL;
    }

    if ((task->type & 1) == 1)
    {
        do_log(LOG_DEBUG,
               "task [%d] fd [%d] do read!",
               task->type, task->fd);
        if (fnc_read)
        {
            mod_fd = fnc_read(rena, task, &cp);
        }
    } else if (task->type > 0)
    {
        do_log(LOG_DEBUG,
               "task [%d] fd [%d] do write!",
               task->type, task->fd);
        if (fnc_write)
        {
            mod_fd = fnc_write(rena, task, &cp);
        }
    } else {
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

    do_log(LOG_DEBUG, "set forced exit to 1");
    rena->forced_exit = 1;
}
