#ifndef SERVER_H_
#define SERVER_H_

struct server *server_init(struct rena *);
void server_destroy(struct rena *);
int server_notify(struct rena *, int op, int fd, int mask);
int server_dispatch(struct rena *);
int server_fd_nonblock(int fd);
int server_receive_client(struct rena *, int fd, void **ssl);

#endif
