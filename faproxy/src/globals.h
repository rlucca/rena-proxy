#pragma once


#define MAX_JOBS 256
#define MAX_STR 1024
#define MAX_FDS 1024
#define HTTP_PORT 80
#define HTTPS_PORT 443


#include <ev.h>
#include <stdio.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include "request.h"
#include "queue.h"
#include "ssl.h"


typedef struct {
    struct ev_loop *main_loop;
    ev_io http_watcher;
    ev_io https_watcher;
    ev_async wake_up;
    //ev_io write_watcher; // LATER?
    //ev_stat config_watcher; // LATER
    
    struct in_addr *bind_addr;
    int http_fd; // esses sockets ficam em passivo
    int https_fd;
    SSL_CTX *ssl_ctx; // LATER should use a BIO ctx?

    char server_suffix[MAX_STR];

    // jobs is controlled by job_queue
    // and job_queue is allocated on stack
    queue_payload_t *jobs[256]; // deveria isso ser alterado em proporcao ao MAX_FDS?
    queue_t *job_queue;
} fa_control_t;

extern pthread_t *threads;
extern size_t n_threads;
extern fa_request_t *requests[MAX_FDS];
extern fa_control_t events;

extern fa_request_t *accept_action_internal(char is_ssl, int client, int side);
extern void *worker_thread(void *);
extern int setnonblock(int fd);
extern int prepare_socket(int port);
extern int connect_to_server(const char *host, int port);

#define DEBUG_MODE_ON 1
#if DEBUG_MODE_ON
#define DEBUG(msg,...) fprintf(stderr, "[%s:%s:%d] " msg "\n", \
                               __FILE__, __FUNCTION__, __LINE__, \
                              __VA_ARGS__+0)
#else
#define DEBUG(msg,...) ;
#endif

#define DECLARE_ENQUEUE_ACTION(fname, action, is_https)                 \
static void fname(struct ev_loop *loop, struct ev_io *w, int revents)   \
{                                                                       \
    DEBUG("event data [%p] fd [%d] re [%d]", loop, w->fd, revents);     \
    ev_io_stop(events.main_loop, w);                                    \
    queue_payload_t *job = queue_payload(action, w->fd, is_https);      \
    queue_enqueue(events.job_queue, job);                               \
}

#define RESTART_EV_IO(watcher)                                          \
    ev_io_start(events.main_loop, watcher);                             \
    ev_async_send(events.main_loop, &events.wake_up)
