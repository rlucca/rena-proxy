#ifndef SERVER_H_
#define SERVER_H_

struct server *server_init(struct rena *);
void server_destroy(struct rena *);
int server_notify(struct rena *, int op, int fd, int mask);
int server_dispatch(struct rena *);
int server_receive_client(struct rena *, int fd, void **ssl);
int server_handshake_client(int fd, void *is_ssl);
int server_socket_for_client(struct rena *rena, void *address);
int server_tcp_connection_done(int fd);

int server_address_from_host(const char *host, void **out);
void server_address_set_port(void *address, int port);

int server_read_client(int fd, void *is_ssl, void *out, size_t *out_len);
int server_write_client(int fd, void *is_ssl, void *out, size_t *out_len);

int server_set_client_as_secure(struct rena *, void *peer);
int server_try_client_connect(struct rena *, void *peer);
int server_client_set_ssl_data(struct rena *, void *data, int fd);
int server_update_notify(struct rena *rena, int fd, int w, int r);

#endif
