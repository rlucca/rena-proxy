#include "global.h"
#include "task_manager.h"

#include <stdlib.h>

typedef struct task_manager {
    int min_tasks;
    int max_tasks;
    int number_of_tasks;
    struct task **tasks;
} task_manager_t;


struct task_manager *task_manager_init(struct rena *rena)
{
    rena->tm = calloc(1, sizeof(struct task_manager));
    config_get_pool_minimum(&rena->config,
                            &rena->tm->min_tasks);
    config_get_pool_maximum(&rena->config,
                            &rena->tm->max_tasks);

    do_log(LOG_DEBUG, "poll size to [%d, %d]",
           rena->tm->min_tasks, rena->tm->max_tasks);

    return rena->tm;
}

void task_manager_run(struct rena *rena)
{
    task_manager_t *tm = rena->tm;
    tm->tasks = calloc(tm->min_tasks, sizeof(struct task *));
    for (tm->number_of_tasks=0;
         !rena->exit && tm->number_of_tasks < tm->min_tasks;
         tm->number_of_tasks++)
    {
        struct task *task = task_create(tm->number_of_tasks, rena);
        if (task == NULL)
        {
            rena->exit = 1;
        }
        tm->tasks[tm->number_of_tasks] = task;
    }

    if (rena->exit == 0)
    {
        task_mainthread_work(rena);
    }
}

void task_manager_destroy(struct rena *rena)
{
    task_manager_t *tm = rena->tm;
    for (int i=0; i < tm->number_of_tasks; i++)
    {
        task_release(i, &tm->tasks[i]);
    }

    free(tm->tasks);
    free(rena->tm);
    rena->tm = NULL;
}
