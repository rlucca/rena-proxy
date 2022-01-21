#ifndef SERVER_H_
#define SERVER_H_

struct server *server_init(struct rena *);
void server_destroy(struct rena *);
int server_notify(struct rena *, int op, int fd, int mask);
int server_dispatch(struct rena *);
int server_receive_client(struct rena *, int fd, void **ssl);
int server_handshake_client(int fd, void *is_ssl);
int server_socket_for_client(struct rena *rena);

int server_address_from_host(const char *host, void **out);
void server_address_set_port(void *address, int port);
void server_address_next(void *address, void **out);
void server_address_free(void *address);

int server_read_client(int fd, void *is_ssl, void *out, size_t *out_len);
int server_write_client(int fd, void *is_ssl, void *out, size_t *out_len);

int server_client_connect(struct rena *, void *address, void *is_ssl);

#endif
