#ifndef SERVER_H_
#define SERVER_H_

struct server *server_init(struct rena *);
void server_destroy(struct rena *);
int server_notify(struct rena *, int op, int fd, int mask);

#endif
