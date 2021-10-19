#ifndef TASK_H_
#define TASK_H_

struct task;

struct task *task_create(int, struct rena *);
void task_release(int, struct task **);
void task_mainthread_work(struct rena *);

#endif
