#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include "task.h"

struct task_manager *task_manager_init(struct rena *);
void task_manager_run(struct rena *);
void task_manager_destroy(struct rena *);

#endif
