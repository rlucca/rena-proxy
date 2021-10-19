
#include "global.h"
#include "task_manager.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

typedef struct task {
    int idle;
    int accepting;
    time_t last_change;
    pthread_t tid;
} task_t;

static void *dummy(void *arg) {
struct rena *rena = (struct rena *) arg;
for (;rena->exit == 0;) pause(); // trava na espera de sinal...
return NULL; }

struct task *task_create(int number,
                         struct rena *rena)
{
    struct task *ret = calloc(1, sizeof(struct task));
    if (number == 0)
    {
        ret->tid = pthread_self();
    } else {
        if (pthread_create(&ret->tid, NULL,
                           &dummy, rena) != 0)
        {
            do_log(LOG_ERROR,
                   "pthread_create returned -1 (%m)");
            free(ret);
            return NULL;
        }
    }

    ret->idle = 1;
    ret->accepting = 1;
    ret->last_change = time(NULL);
    return ret;
}

void task_release(int number, struct task **task)
{
    if (task == NULL || *task == NULL)
    {
        return ;
    }

    if (number != 0)
    {
        pthread_join((*task)->tid, NULL);
    }

    free(*task);
    *task = NULL;
}

void task_mainthread_work(struct rena *rena)
{
    dummy(rena);
}
