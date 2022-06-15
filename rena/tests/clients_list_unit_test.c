
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#ifndef CLIENTS_H_
#define http_find_header http_find_header_dummy
#define http_header_value http_header_value_dummy
#define http_request_line http_request_line_dummy
#define http_status http_status_dummy
#define http_bytes_sent http_bytes_sent_dummy
#include "clients.c"
#endif

#include <fcntl.h>

CHEAT_DECLARE(
    struct clients *cs = NULL;

    int http_find_header_called = 0;
    int http_find_header_ret = -1;
    int http_find_header_dummy(void *cp, const char *n, int n_len)
    {
        (void) cp;
        (void) n;
        (void) n_len;
        http_find_header_called++;
        return http_find_header_ret;
    }

    int http_header_value_called = 0;
    int http_header_value_ret = -1;
    int http_header_value_dummy(void *cp, char *n, int n_len, int i)
    {
        (void) cp;
        (void) n;
        (void) n_len;
        (void) i;
        http_header_value_called++;
        return http_header_value_ret;
    }

    int http_request_line_called = 0;
    int http_request_line_ret = -1;
    int http_request_line_dummy(void *cp, char *n, int n_len)
    {
        (void) cp;
        (void) n;
        (void) n_len;
        http_request_line_called++;
        return http_request_line_ret;
    }

    int http_status_called = 0;
    int http_status_ret = -1;
    int http_status_dummy(void *cp, char *n, int n_len)
    {
        (void) cp;
        (void) n;
        (void) n_len;
        http_status_called++;
        return http_status_ret;
    }

    int http_bytes_sent_called = 0;
    int http_bytes_sent_ret = -1;
    int http_bytes_sent_dummy(void *cp, char *n, int n_len)
    {
        (void) cp;
        (void) n;
        (void) n_len;
        http_bytes_sent_called++;
        return http_bytes_sent_ret;
    }
)

CHEAT_SET_UP(
    cs = clients_init();
    cheat_assert(cs != NULL);
    cheat_assert(cs->cci == NULL);

    http_find_header_called = 0;
    http_find_header_ret = -1;
    http_header_value_called = 0;
    http_header_value_ret = -1;
    http_request_line_called = 0;
    http_request_line_ret = -1;
    http_status_called = 0;
    http_status_ret = -1;
    http_bytes_sent_called = 0;
    http_bytes_sent_ret = -1;
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
    clients_set_ssl_state(&aux, on);
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
    clients_add(cs, REQUESTER_TYPE, fd_to_dup);
    clients_add(cs, REQUESTER_TYPE, fds[1]);
    clients_add(cs, REQUESTER_TYPE, fds[0]);
    cheat_assert(cs->cci->next->requester->fd == fds[1]);
    client_position_t aux;
    int ret = clients_search(cs, fds[1], &aux);
    cheat_assert(ret == 0);
    cheat_assert(aux.info != NULL);
    cheat_assert(aux.pos != NULL);
    cheat_assert(aux.type == REQUESTER_TYPE);
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


CHEAT_TEST(clients_log_format_invalid,

    cheat_assert(client_do_log_format(NULL) == -1);

    client_log_format_t invalid;
    invalid.type = LOG_FORMAT_LAST + 4;
    cheat_assert(client_do_log_format(&invalid) == -1);
)

CHEAT_TEST(clients_log_format_bytes_sent,

    char temp[128];
    struct client_info vit;
    struct circle_client_info cci = { NULL, NULL, NULL, &vit };
    client_position_t cp = { NULL, INVALID_TYPE, &cci };
    client_log_format_t clf = { LOG_FORMAT_BYTES_RECEIVED, &cp, NULL, temp,
                                sizeof(temp), 0 };
    int ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, -1);
    cheat_assert_int32(http_bytes_sent_called, 1);

    http_bytes_sent_ret = 44;
    ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, 44);
    cheat_assert_int32(http_bytes_sent_called, 2); // 1 + 1
)

CHEAT_TEST(clients_log_format_status_code,

    char temp[128];
    struct client_info vit;
    struct circle_client_info cci = { NULL, NULL, NULL, &vit };
    client_position_t cp = { NULL, INVALID_TYPE, &cci };
    client_log_format_t clf = { LOG_FORMAT_STATUS_CODE, &cp, NULL, temp,
                                sizeof(temp), 0 };
    int ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, -1);
    cheat_assert_int32(http_status_called, 1);

    http_status_ret = 44;
    ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, 44);
    cheat_assert_int32(http_status_called, 2); // 1 + 1
)

CHEAT_TEST(clients_log_format_ip_client,

    const char ip[] = "666.66.666.666";
    struct client_info req;
    memcpy(req.ip, ip, sizeof(ip) - 1);
    struct circle_client_info cci = { NULL, NULL, &req, NULL };
    client_position_t cp = { NULL, INVALID_TYPE, &cci };
    char temp[128];
    client_log_format_t ok;
    ok.type = LOG_FORMAT_REQUEST_IP_CLIENT;
    ok.client = &cp;
    ok.out = temp;
    ok.out_sz = sizeof(temp);

    int ret = client_do_log_format(&ok);
    cheat_assert_int32(ret, sizeof(ip) - 1);
    cheat_assert_string(temp, ip);
)

CHEAT_TEST(clients_log_format_first_line,

    char temp[128];
    struct client_info req;
    struct circle_client_info cci = { NULL, NULL, &req, NULL };
    client_position_t cp = { NULL, INVALID_TYPE, &cci };
    client_log_format_t clf = { LOG_FORMAT_REQUEST_LINE, &cp, NULL, temp,
                                sizeof(temp), 0 };
    int ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, -1);
    cheat_assert_int32(http_request_line_called, 1);

    http_request_line_ret = 44;
    ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, 44);
    cheat_assert_int32(http_request_line_called, 2); // 1 + 1
)

CHEAT_TEST(clients_log_format_header_value,

    char temp[128];
    struct client_info req;
    struct client_info vit;
    struct circle_client_info cci = { NULL, NULL, &req, &vit };
    client_position_t cp = { NULL, INVALID_TYPE, &cci };
    client_log_format_t clf = { LOG_FORMAT_HEADER_VALUE, &cp, NULL, temp,
                                sizeof(temp), 0 };
    int ret = client_do_log_format(&clf);
    cheat_assert_int32(ret, -1);
    cheat_assert_int32(http_find_header_called, 0);
    cheat_assert_int32(http_header_value_called, 0);

    const char a[] = "attribute";
    client_log_format_t okay = { LOG_FORMAT_HEADER_VALUE, &cp, a, temp,
                                sizeof(temp), sizeof(a) - 1 };
    ret = client_do_log_format(&okay);
    cheat_assert_int32(ret, -2);
    cheat_assert_int32(http_find_header_called, 2);
    cheat_assert_int32(http_header_value_called, 0);

    http_find_header_ret = 1;
    ret = client_do_log_format(&okay);
    cheat_assert_int32(ret, -2);
    cheat_assert_int32(http_find_header_called, 3); // 2 + 1
    cheat_assert_int32(http_header_value_called, 1); // 0 + 1

    http_header_value_ret = 44;
    ret = client_do_log_format(&okay);
    cheat_assert_int32(ret, 44);
    cheat_assert_int32(http_find_header_called, 4); // 3 + 1
    cheat_assert_int32(http_header_value_called, 2); // 1 + 1
)
