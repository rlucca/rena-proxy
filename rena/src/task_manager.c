#include "global.h"
#include "task_manager.h"

struct task_manager *task_manager_init(struct rena *rena)
{
    return (void *) 0x01;
}

void task_manager_run(struct rena *rena)
{
    (void) rena;
}

void task_manager_destroy(struct rena *rena)
{
    (void) rena;
}
