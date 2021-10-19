
#include "global.h"
#include "task_manager.h"

struct task *task_create(int number,
                         struct rena *rena)
{
    return NULL;
}

void task_release(int number, struct task **task)
{
    (void) number;
    (void) task;
}

void task_mainthread_work(struct rena *rena)
{
    (void) rena;
}
