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
    float addictive_ratio;
    int reap_time;
    pthread_t *tasks;
    void **queue_internal;
    queue_t *queue;
} task_manager_t;

static void queue_init(struct task_manager *tm)
{
    int mintm = tm->max_tasks * 1024;
    int minma = 4096;
    int minimal = (minma > mintm) ? minma : mintm;
    queue_t queue = { NULL, minimal, 0, 0, 0,
                      PTHREAD_MUTEX_INITIALIZER,
                      PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };
    tm->queue_internal = calloc(queue.capacity, sizeof(void *));
    queue.buffer = tm->queue_internal;
    tm->queue = malloc(sizeof(queue_t));
    memcpy(tm->queue, &queue, sizeof(queue_t));
}

static void queue_destroy(struct task_manager *tm)
{
    pthread_cond_destroy(&tm->queue->cond_full);
    pthread_cond_destroy(&tm->queue->cond_empty);
    pthread_mutex_destroy(&tm->queue->mutex);
    free(tm->queue);
    free(tm->queue_internal);
    tm->queue = NULL;
}

void task_manager_task_drop_fd(struct task_manager *tm, int fd)
{
    int many = 0;
    int pos = -1;

    if (tm == NULL)
        return ;

    do_log(LOG_DEBUG, "trying drop tasks from fd [%d]..." , fd);
    THREAD_CRITICAL_BEGIN(tm->queue->mutex);
    pos = tm->queue->out;
    while (pos != tm->queue->in)
    {
        task_t *ptr = (task_t *) tm->queue->buffer[pos];
        if (ptr != NULL && ptr->fd == fd)
        {
            ptr->fd = -1;
            many++;
        }
        pos = (pos + 1) % tm->queue->capacity;
    }
    THREAD_CRITICAL_END(tm->queue->mutex);
    do_log(LOG_DEBUG, "all tasks [%d] dropped from fd [%d]" , many, fd);
}

struct task_manager *task_manager_init(struct rena *rena)
{
    rena->tm = calloc(1, sizeof(struct task_manager));
    config_get_pool_minimum(&rena->config,
                            &rena->tm->min_tasks);
    config_get_pool_maximum(&rena->config,
                            &rena->tm->max_tasks);
    config_get_pool_addictive(&rena->config,
                            &rena->tm->addictive_ratio);
    config_get_pool_reap_time(&rena->config,
                            &rena->tm->reap_time);
    rena->tm->number_of_working_tasks = rena->tm->max_tasks;
    rena->tm->last_peak = time(NULL);

    if (rena->tm->min_tasks <= 1 || rena->tm->min_tasks > rena->tm->max_tasks)
    {
        do_log(LOG_ERROR,
               "poll size with a invalid poll size, minimal need be greater "
               "than 1 and minimal need be less than or equal to maximum");
        free(rena->tm);
        rena->tm = NULL;
        return NULL;
    }

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
    if (!tm)
    {
        return ;
    }

    for (int i=1; i < tm->number_of_tasks; i++)
    {
        if (pthread_join(tm->tasks[i], NULL))
        {
            do_log(LOG_ERROR, "pthread_join (%d:%lu) failed -- %m",
                   i, tm->tasks[i]);
        }
    }

    task_runner_destroy(rena);

    free(tm->tasks);
    queue_destroy(tm);
    free(rena->tm);
    rena->tm = NULL;
}

int task_manager_task_queue_size(struct rena *rena)
{
    struct task_manager *tm = rena->tm;
    return queue_size(tm->queue);
}

void task_manager_task_push(struct rena *rena, int fd, task_type_e tt)
{
    struct task_manager *tm = rena->tm;
    task_t *task = NULL;

    if (tt == TT_INVALID)
    {
        task_manager_task_drop_fd(tm, fd);
    }

    task = calloc(sizeof(task_t), 1);
    task->type = tt;
    task->fd = fd;
    queue_enqueue(tm->queue, task);
}

task_t *task_manager_task_consume(struct rena *rena)
{
    struct task_manager *tm = rena->tm;
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

    proc_raise(SIGHUP);
}

void task_manager_can_notify_change_of_tasks(struct rena *rena, double ratio)
{
    time_t now = time(NULL);
    struct task_manager *tm = rena->tm;
    if (now <= tm->last_peak + 5)
        return ;

    if (ratio >= tm->addictive_ratio)
    {
        THREAD_CRITICAL_BEGIN(lock)
        if (tm->number_of_tasks < tm->max_tasks)
            proc_raise(SIGTTIN);
        THREAD_CRITICAL_END(lock)
        tm->last_peak = time(NULL);
    } else { // ratio < tm->addictive_ratio
        if (now > tm->last_peak + tm->reap_time)
        {
            THREAD_CRITICAL_BEGIN(lock)
            if (tm->number_of_tasks > tm->min_tasks)
                proc_raise(SIGTTOU);
            THREAD_CRITICAL_END(lock)
            tm->last_peak = time(NULL);
        }
    }
}
