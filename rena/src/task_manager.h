#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

typedef enum task_type {
    TT_INVALID = 0,
    TT_READ = 1,
    TT_WRITE = 2,
    TT_NORMAL_READ = 3,
    TT_NORMAL_WRITE = 4,
    TT_SECURE_READ = 5,
    TT_SECURE_WRITE = 6,
    TT_SIGNAL_READ = 7,
    TT_SIGNAL_WRITE = 8,
} task_type_e;

struct task;
struct client_position;

typedef int (*handle_method_t)(struct rena *,
                               struct task *,
                               struct client_position *);

typedef struct task {
    task_type_e type;
    int fd;

    handle_method_t read;
    handle_method_t write;
} task_t;


struct task_manager *task_manager_init(struct rena *);
void task_manager_run(struct rena *);
void task_manager_destroy(struct rena *);

int task_manager_task_queue_size(struct rena *rena);
void task_manager_task_push(struct rena *, int fd, task_type_e tt);
task_t *task_manager_task_consume(struct rena *);
void task_manager_task_free(task_t **);
void task_manager_task_drop_fd(struct task_manager *, int fd);

void task_manager_set_working(struct rena *, int flag);
void task_manager_forced_exit(struct rena *rena);


#define THREAD_CORRUPT(K, W)                            \
    if (K(&W) < 0) {                                    \
        text_t buf;                                     \
        proc_errno_message(&buf);                       \
        do_log(LOG_ERROR, "%s fail: %s", #K, buf.text); \
    }
#define THREAD_CRITICAL_BEGIN(X) THREAD_CORRUPT(pthread_mutex_lock, X)
#define THREAD_CRITICAL_END(X) THREAD_CORRUPT(pthread_mutex_unlock, X)

#endif
