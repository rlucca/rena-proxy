#ifndef HTTP_H_
#define HTTP_H_

#include "clients.h"

int http_pull(struct rena *, client_position_t *client, int fd);
int http_push(struct rena *, client_position_t *client, int fd);
int http_evaluate(client_position_t *client);
void http_destroy(void *handler);

#endif
