#include "global.h"
#include "task_manager.h"
#include "task_runner.h"

#include <pthread.h>
#include <stdlib.h>
#include <time.h>

typedef struct task_manager {
    int min_tasks;
    int max_tasks;
    int number_of_tasks;
    int number_of_working_tasks;
    time_t last_peak;
    pthread_t *tasks;
} task_manager_t;


struct task_manager *task_manager_init(struct rena *rena)
{
    rena->tm = calloc(1, sizeof(struct task_manager));
    config_get_pool_minimum(&rena->config,
                            &rena->tm->min_tasks);
    config_get_pool_maximum(&rena->config,
                            &rena->tm->max_tasks);
    rena->tm->number_of_working_tasks = rena->tm->max_tasks;
    rena->tm->last_peak = time(NULL);

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
        if (pthread_join(tm->tasks[tm->number_of_tasks], NULL))
        {
            do_log(LOG_ERROR, "pthread_join (i) failed -- %m");
        }
    }

    free(tm->tasks);
    free(rena->tm);
    rena->tm = NULL;
}
