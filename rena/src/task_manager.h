#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

typedef enum task_type {
    TT_INVALID = 0,
    TT_READ = 1,
    TT_WRITE = 2,
} task_type_e;

typedef struct {
    task_type_e type;
    int fd;
} task_t;


struct task_manager *task_manager_init(struct rena *);
void task_manager_run(struct rena *);
void task_manager_destroy(struct rena *);

void task_manager_task_push(struct rena *, int fd, task_type_e tt);
task_t *task_manager_task_consume(struct rena *);
void task_manager_task_free(task_t **);

void task_manager_set_working(struct rena *, int flag);
void task_manager_forced_exit(struct rena *rena);


#define THREAD_CORRUPT(K, W)                  \
    if (K(&W) < 0) {                          \
        char buf[MAX_STR];                    \
        proc_errno_message(buf, sizeof(buf)); \
        do_log(LOG_ERROR, "%s_fail", #K);     \
    }
#define THREAD_CRITICAL_BEGIN(X) THREAD_CORRUPT(pthread_mutex_lock, X)
#define THREAD_CRITICAL_END(X) THREAD_CORRUPT(pthread_mutex_unlock, X)

#endif
