#include "global.h"
#include "task_manager.h"
#include "task_runner.h"
#include "proc.h"
#include "queue.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct task_manager {
    int min_tasks;
    int max_tasks;
    int number_of_tasks;
    int number_of_working_tasks;
    time_t last_peak;
    pthread_t *tasks;
    void *queue_internal[4096];
    queue_t *queue;
} task_manager_t;

static void queue_init(struct task_manager *tm)
{
    queue_t queue = QUEUE_INITIALIZER(tm->queue_internal);
    tm->queue = malloc(sizeof(queue_t));
    memcpy(tm->queue, &queue, sizeof(queue_t));
}

static void queue_destroy(struct task_manager *tm)
{
    pthread_cond_destroy(&tm->queue->cond_full);
    pthread_cond_destroy(&tm->queue->cond_empty);
    pthread_mutex_destroy(&tm->queue->mutex);
    free(tm->queue);
    tm->queue = NULL;
}

struct task_manager *task_manager_init(struct rena *rena)
{
    rena->tm = calloc(1, sizeof(struct task_manager));
    config_get_pool_minimum(&rena->config,
                            &rena->tm->min_tasks);
    config_get_pool_maximum(&rena->config,
                            &rena->tm->max_tasks);
    rena->tm->number_of_working_tasks = rena->tm->max_tasks;
    rena->tm->last_peak = time(NULL);
    queue_init(rena->tm);

    do_log(LOG_DEBUG, "poll size to [%d, %d]",
           rena->tm->min_tasks, rena->tm->max_tasks);

    return rena->tm;
}

void task_manager_run(struct rena *rena)
{
    task_manager_t *tm = rena->tm;
    tm->tasks = calloc(tm->min_tasks, sizeof(pthread_t));
    for (tm->number_of_tasks=0;
         !rena->forced_exit && tm->number_of_tasks < tm->min_tasks;
         tm->number_of_tasks++)
    {
        if (tm->number_of_tasks == 0)
        {
            tm->tasks[tm->number_of_tasks] = pthread_self();
        } else {
            int ret = pthread_create(&tm->tasks[tm->number_of_tasks],
                                     NULL, &task_runner, rena);
            if (ret != 0)
            {
                do_log(LOG_ERROR, "pthread_create failed -- %m");
                rena->forced_exit = 1;
            }
        }
    }

    if (rena->forced_exit == 0)
    {
        rena->tm->number_of_working_tasks = 0;
        task_runner(rena);
    }
}

void task_manager_destroy(struct rena *rena)
{
    task_manager_t *tm = rena->tm;
    for (int i=1; i < tm->number_of_tasks; i++)
    {
        if (pthread_join(tm->tasks[i], NULL))
        {
            do_log(LOG_ERROR, "pthread_join (%d:%lu) failed -- %m",
                   i, tm->tasks[i]);
        }
    }
    free(tm->tasks);
    queue_destroy(tm);
    free(rena->tm);
    rena->tm = NULL;
}

void task_manager_task_push(struct rena *rena, int fd, task_type_e tt)
{
    struct task_manager *tm = rena->tm;
    task_t *task = malloc(sizeof(task_t));
    task->type = tt;
    task->fd = fd;
    queue_enqueue(tm->queue, task);
}

task_t *task_manager_task_consume(struct rena *rena)
{
    struct task_manager *tm = rena->tm;
    do_log(LOG_DEBUG, "waiting task");
    return (task_t *)queue_dequeue(tm->queue);
}

void task_manager_task_free(task_t **task)
{
    if (task && *task)
    {
        free(*task);
        *task = NULL;
    }
}

void task_manager_set_working(struct rena *rena, int flag)
{
    THREAD_CRITICAL_BEGIN(lock)

    if (flag != 0)
        rena->tm->number_of_working_tasks++;
    else
        rena->tm->number_of_working_tasks--;

    THREAD_CRITICAL_END(lock)
}

void task_manager_forced_exit(struct rena *rena)
{
    struct task_manager *tm = rena->tm;

    if (rena->forced_exit == 0)
        return ;

    for (int n=0; n < tm->number_of_tasks; n++)
        queue_enqueue(tm->queue, NULL);

    proc_raise(1);
}
