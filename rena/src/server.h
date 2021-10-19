#ifndef SERVER_H_
#define SERVER_H_

struct server; // forward
struct rena;

struct server *server_init(struct rena *modules);
void server_destroy(struct rena *modules);

#endif
