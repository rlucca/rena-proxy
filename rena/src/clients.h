#ifndef CLIENTS_H_
#define CLIENTS_H_

struct rena;
struct clients;
struct client_info;

typedef enum {
    INVALID_TYPE = 0,
    REQUESTER_TYPE,
    VICTIM_TYPE
} client_type_e;

typedef struct client_position {
    const struct client_info *info;
    client_type_e type;
    const void *pos;
} client_position_t;

struct clients *clients_init();
void clients_destroy(struct clients **);

int clients_quantity(struct clients *);
int clients_add(struct clients *, client_type_e t, int fd);
int clients_del(struct clients *, client_position_t *);
int clients_search(struct clients *, int fd,
                   client_position_t *out); // -1 error, 0 found, 1 not found
void clients_set_tcp(client_position_t *, int state);
void clients_set_ssl(client_position_t *, void *ssl);
void clients_set_ssl_state(client_position_t *p, int state);
void clients_set_fd(client_position_t *p, int);
void clients_set_working(client_position_t *, int state);
void clients_set_protocol(client_position_t *, void *);
void clients_set_userdata(client_position_t *, void *);
void clients_set_handshake(client_position_t *, int);
void clients_set_want(client_position_t *, char Iwant, char SSLwant);
void clients_clear_want(client_position_t *);
int clients_get_want(client_position_t *);
int clients_add_peer(client_position_t *, int fd);
int clients_get_peer(client_position_t *, client_position_t *out);
int clients_get_fd(client_position_t *);
int clients_get_tcp(client_position_t *);
int clients_get_working(client_position_t *);
int clients_get_ssl_state(client_position_t *);
void *clients_get_ssl(client_position_t *);
const char *clients_get_ip(client_position_t *);
void *clients_get_protocol(client_position_t *);
void *clients_get_userdata(client_position_t *);
int clients_get_handshake(client_position_t *);
int clients_protocol_lock(client_position_t *p, int change_too);
int clients_protocol_unlock(client_position_t *p, int change_too);

int client_do_read(struct rena *, client_position_t *, int fd);
int client_do_write(struct rena *, client_position_t *, int fd);

#endif
