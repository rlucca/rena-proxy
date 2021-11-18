#ifndef CLIENTS_H_
#define CLIENTS_H_

struct clients;
struct client_info;

typedef enum {
    INVALID_TYPE = 0,
    REQUESTER_TYPE,
    VICTIM_TYPE
} client_type_e;

typedef struct {
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
void clients_set_working(client_position_t *, int state);
int clients_add_peer(client_position_t *, int fd);
int clients_get_peer(client_position_t *, client_position_t *out);
int clients_get_fd(client_position_t *);
int clients_get_tcp(client_position_t *);
int clients_get_working(client_position_t *);
int clients_get_ssl_state(client_position_t *);
void *clients_get_ssl(client_position_t *);
const char *clients_get_ip(client_position_t *);

#endif
