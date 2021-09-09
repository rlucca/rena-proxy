#pragma once

#include <time.h>

enum {
    STATE_UNKNOWN = 0,
    STATE_READ = 1,
    STATE_WRITE = 2
};

typedef struct request {
    int fd; // socket
    int related_fd; // server socket, if handling client; client socket if handling server
    ev_io read_watcher;
    ev_io write_watcher;
    SSL *ctx;
    int side; // handle client (-1) or handle server (1) or unknown

    int curr_state; // 1 read 2 write 0 waiting (get mutex to change or check)
    void *html_state_data; //<<TODO hdr referer|user-agent, method, path, protocol, status

    //char user_name[MAX_STR];
    char user_ip[MAX_STR];
    time_t creation_at; // no timezone
    size_t n_bytes;

    // communication window-buffer
    int payload_r;
    int payload_w;
    int payload_size;
    char payload[MAX_STR];

    // Hold it to change!
    pthread_mutex_t mutex;
} fa_request_t;

fa_request_t *fa_request_create(int client, SSL *ssl, int side);
int fa_request_curr_state(fa_request_t *fa);
void fa_request_curr_state_set(fa_request_t *fa, int cs);
size_t fa_request_remain_capacity(fa_request_t *fa);
size_t fa_request_size(fa_request_t *fa);
fa_request_t *fa_request_push_holding(fa_request_t *fa);
int fa_request_payload_transform(fa_request_t *fa,
                           const char *buffer, size_t buffer_sz);
int fa_request_payload_put(fa_request_t *fa,
                           const char *buffer, size_t buffer_sz);
int fa_request_payload_copy(fa_request_t *fa,  // not consume
                           char *buffer, size_t buffer_sz);
void fa_request_payload_consume(fa_request_t *fa, size_t buffer_sz);
/*sync*/ int fa_request_delete(fa_request_t *fa);
void fa_request_close_socket(int fd);
