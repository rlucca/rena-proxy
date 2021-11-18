
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#ifndef CLIENTS_H_
#include "clients.c"
#endif

#include <fcntl.h>

CHEAT_DECLARE(
    struct clients *cs = NULL;
)

CHEAT_SET_UP(
    cs = clients_init();
    cheat_assert(cs != NULL);
    cheat_assert(cs->cci == NULL);
)

CHEAT_TEAR_DOWN(
    clients_destroy(&cs);
    cheat_assert(cs == NULL);
)

CHEAT_TEST(clients_init_destroy, )

CHEAT_TEST(clients_add,
    int fd_to_dup=open("/dev/null",'r');
    int p = clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    cheat_assert(p == 0);
    cheat_assert(cs->cci != NULL);
    cheat_assert(cs->cci->next != NULL);
    cheat_assert(cs->cci->prev != NULL);
    cheat_assert(cs->cci->next == cs->cci->prev);
    cheat_assert(cs->cci->victim == NULL);
    cheat_assert(cs->cci->requester != NULL);
    cheat_assert(cs->cci->requester->ssl == NULL);
    cheat_assert(cs->cci->requester->fd == fd_to_dup);
    cheat_assert(cs->cci->requester->working == 0);
)

CHEAT_TEST(clients_sequencial_add,
    int fd_to_dup=open("/dev/null",'r');
    int fds[2] = { dup(fd_to_dup), dup(fd_to_dup) };
    int p = clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    int x = clients_add(cs, REQUESTER_TYPE, fds[0]);
    int z = clients_add(cs, REQUESTER_TYPE, fds[1]);
    cheat_assert(p == 0);
    cheat_assert(x == 0);
    cheat_assert(z == 0);
    cheat_assert(cs->cci != NULL);
    cheat_assert(cs->cci->next != NULL);
    cheat_assert(cs->cci->prev != NULL);
    cheat_assert(cs->cci->next->next->next == cs->cci);
    cheat_assert(cs->cci->victim == NULL);
    cheat_assert(cs->cci->requester != NULL);
    cheat_assert(cs->cci->requester->ssl == NULL);
    cheat_assert(cs->cci->requester->working == 0);
    cheat_assert(cs->cci->requester->fd == fds[1]);
    cheat_assert(cs->cci->next->requester->fd == fds[0]);
    cheat_assert(cs->cci->next->next->requester->fd == fd_to_dup);
    cheat_assert(cs->cci->next->next->requester->fd == fd_to_dup);
)

CHEAT_TEST(clients_search_requester,
    int fd_to_dup=open("/dev/null",'r');
    int fds[2] = { dup(fd_to_dup), dup(fd_to_dup) };
    int p = clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    int x = clients_add(cs, REQUESTER_TYPE, fds[0]);
    int z = clients_add(cs, REQUESTER_TYPE, fds[1]);
    cheat_assert(p == 0);
    cheat_assert(x == 0);
    cheat_assert(z == 0);
    cheat_assert(cs->cci != NULL);
    client_position_t aux = { (const struct client_info *) 1, VICTIM_TYPE,
                              (const void *) 1};
    int ret = clients_search(cs, INT_MAX, &aux);
    cheat_assert(ret == 1);
    cheat_assert(aux.info == NULL);
    cheat_assert(aux.pos == NULL);
    cheat_assert(aux.type == INVALID_TYPE);
    ret = clients_search(cs, fds[0], &aux);
    cheat_assert(ret == 0);
    cheat_assert(aux.info
                 == (const struct client_info *) cs->cci->next->requester);
    cheat_assert(aux.pos == (const void *) cs->cci->next);
    cheat_assert(aux.type == REQUESTER_TYPE);
)
CHEAT_TEST(clients_info_setters_getters,
    int fd_to_dup=open("/dev/null",'r');
    int fds[3] = { dup(fd_to_dup), dup(fd_to_dup), dup(fd_to_dup) };
    clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    clients_add(cs, REQUESTER_TYPE, fds[1]);
    clients_add(cs, REQUESTER_TYPE, fds[0]);
    client_position_t aux;
    int ret = clients_search(cs, fds[0], &aux);
    cheat_assert(ret == 0);
    cheat_assert(aux.info != NULL);
    cheat_assert(aux.pos != NULL);
    int on = 1;
    clients_set_tcp(&aux, on);
    clients_set_working(&aux, on);
    clients_set_ssl(&aux, (void *) &on);
    cheat_assert(cs->cci->requester->fd==fds[0]);
    cheat_assert(cs->cci->requester->tcp_connected==on);
    cheat_assert(cs->cci->requester->working==on);
    cheat_assert(cs->cci->requester->ssl_connected==on);
    cheat_assert(cs->cci->requester->ssl==(SSL *)&on);
    int z = clients_add_peer(&aux, fds[2]);
    cheat_assert(z == 0);
    cheat_assert(cs->cci->requester != NULL);
    cheat_assert(cs->cci->victim != NULL);
    cheat_assert(cs->cci->requester->fd == fds[0]);
    cheat_assert(cs->cci->victim->fd == fds[2]);
    cheat_assert(clients_get_fd(&aux) == fds[0]);
    cheat_assert(clients_get_tcp(&aux) == on);
    cheat_assert(clients_get_ssl_state(&aux) == on);
    cheat_assert(clients_get_working(&aux) == on);
    cheat_assert(clients_get_ssl(&aux) == (void *) &on);
    cheat_assert(clients_get_ip(&aux) != NULL);
    client_position_t biz;
    int k = clients_get_peer(&aux, &biz);
    cheat_assert(k == 0);
    cheat_assert(clients_get_fd(&biz) == fds[2]);
    cheat_assert(biz.type == VICTIM_TYPE);
    cheat_assert(biz.pos == aux.pos);
    // remove invalid ssl
    cs->cci->requester->ssl = NULL;
)

CHEAT_TEST(clients_del_start,
    int fd_to_dup=open("/dev/null",'r');
    int fds[3] = { dup(fd_to_dup), dup(fd_to_dup), dup(fd_to_dup) };
    clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    clients_add(cs, REQUESTER_TYPE, fds[1]);
    clients_add(cs, REQUESTER_TYPE, fds[0]);
    cheat_assert(cs->cci->requester->fd == fds[0]);
    client_position_t aux;
    int ret = clients_search(cs, fds[0], &aux);
    cheat_assert(ret == 0);
    cheat_assert(aux.info != NULL);
    cheat_assert(aux.pos != NULL);
    void *pos = cs->cci->next;
    int ok = clients_del(cs, &aux);
    cheat_assert(ok == 0);
    cheat_assert(aux.info == NULL);
    cheat_assert(aux.pos == NULL);
    cheat_assert(aux.type == INVALID_TYPE);
    cheat_assert(cs->cci == pos);
)

CHEAT_TEST(clients_del_middle,
    int fd_to_dup=open("/dev/null",'r');
    int fds[3] = { dup(fd_to_dup), dup(fd_to_dup), dup(fd_to_dup) };
    clients_add(cs, VICTIM_TYPE, fd_to_dup);
    clients_add(cs, VICTIM_TYPE, fds[1]);
    clients_add(cs, VICTIM_TYPE, fds[0]);
    cheat_assert(cs->cci->next->victim->fd == fds[1]);
    client_position_t aux;
    int ret = clients_search(cs, fds[1], &aux);
    cheat_assert(ret == 0);
    cheat_assert(aux.info != NULL);
    cheat_assert(aux.pos != NULL);
    cheat_assert(aux.type == VICTIM_TYPE);
    void *pos1 = cs->cci;
    void *pos2 = cs->cci->prev;
    int ok = clients_del(cs, &aux);
    cheat_assert(ok == 0);
    cheat_assert(aux.info == NULL);
    cheat_assert(aux.pos == NULL);
    cheat_assert(aux.type == INVALID_TYPE);
    cheat_assert(cs->cci == pos1);
    cheat_assert(cs->cci->next == pos2);
    cheat_assert(cs->cci->next->prev == pos1);
)

CHEAT_TEST(clients_del_ending,
    int fd_to_dup=open("/dev/null",'r');
    int fds[3] = { dup(fd_to_dup), dup(fd_to_dup), dup(fd_to_dup) };
    clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    clients_add(cs, REQUESTER_TYPE, fds[1]);
    clients_add(cs, REQUESTER_TYPE, fds[0]);
    cheat_assert(cs->cci->prev->requester->fd == fd_to_dup);
    client_position_t aux;
    int ret = clients_search(cs, fd_to_dup, &aux);
    cheat_assert(ret == 0);
    cheat_assert(aux.info != NULL);
    cheat_assert(aux.pos != NULL);
    cheat_assert(aux.type == REQUESTER_TYPE);
    void *pos1 = cs->cci;
    void *pos2 = cs->cci->next;
    int ok = clients_del(cs, &aux);
    cheat_assert(ok == 0);
    cheat_assert(aux.info == NULL);
    cheat_assert(aux.pos == NULL);
    cheat_assert(aux.type == INVALID_TYPE);
    cheat_assert(cs->cci == pos1);
    cheat_assert(cs->cci->next == pos2);
    cheat_assert(cs->cci->prev == pos2);
)

CHEAT_TEST(clients_del_invalid,
    client_position_t aux;
    aux.type = INVALID_TYPE;
    aux.info = NULL;
    aux.pos = NULL;
    int ok = clients_del(cs, &aux);
    cheat_assert(ok != 0);
    cheat_assert(aux.info == NULL);
    cheat_assert(aux.pos == NULL);
    cheat_assert(aux.type == INVALID_TYPE);
)
