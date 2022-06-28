#ifndef HTTP_H_
#define HTTP_H_

#include "clients.h"

int http_pull(struct rena *, client_position_t *client, int fd);
int http_push(struct rena *, client_position_t *client, int fd,
              client_position_t *peer);
int http_evaluate(struct rena *, client_position_t *client);
void http_destroy(void *handler);
int http_sent_done(void *protocol);

int http_bytes_sent(void *, char *out, int out_sz);
int http_status(void *, char *out, int out_sz);
int http_request_line(void *, char *out, int out_sz, void *ssl);
int http_find_header(void *, const char *name, int name_len);
int http_header_value(void *, char *out, int out_sz, int find_header_ret);

#endif
