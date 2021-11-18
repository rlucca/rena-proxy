
#include "global.h"
#include "task_manager.h"
#include "task_runner.h"
#include "server.h"
#include "proc.h"

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
        }

        task_manager_forced_exit(rena);
    }

    return NULL;
}

static void task_handling(struct rena *rena, task_t *task)
{
    do_log(LOG_DEBUG, "task [%p]: not implemented", task);

    if (task == NULL) return ;

    if (server_notify(rena, EPOLL_CTL_MOD, task->fd, EPOLLIN) < 0)
    {
        abort();
    }

    do_log(LOG_DEBUG, "set forced exit to 1");
    rena->forced_exit = 1;
}
