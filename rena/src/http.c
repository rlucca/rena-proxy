#define _GNU_SOURCE
#include "global.h"
#include "http.h"
#include "database.h"
#include "context_full_link.h"
#include "context_html.h"
#include "task_manager.h"
#include "server.h"
#include "md5.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

struct http {
    struct database_object *lookup_tree;
    int headers_used;
    const char **headers;
    int *headers_length;
    int expected_payload;
    char found_delimiters;
    char last_found_delimiters;
    const char *payload;

    size_t total_block;
    size_t buffer_sent;
    size_t buffer_recv;
    size_t buffer_used;
    char buffer[MAX_STR]; // at_least MAX_STR!! CAUTION: not finished in zero!
};

static const char header_content_length[] = "Content-Length";
static int header_content_length_len = sizeof(header_content_length) - 1;
static const char header_content_type[] = "Content-Type";
static int header_content_type_len = sizeof(header_content_type) - 1;
static const char header_content_md5[] = "Content-Md5";
static int header_content_md5_len = sizeof(header_content_md5) - 1;
static const char header_host[] = "Host";
static int header_host_len = sizeof(header_host) - 1;
static const char header_connection[] = "Connection";
static int header_connection_len = sizeof(header_connection) - 1;
static const char accept_encoding[] = "Accept-Encoding";
static int accept_encoding_len = sizeof(accept_encoding) - 1;
static const char accept_ranges[] = "Accept-Ranges";
static int accept_ranges_len = sizeof(accept_ranges) - 1;
static const char header_accept[] = "Accept";
static int header_accept_len = sizeof(header_accept) - 1;
static const char header_cookie[] = "Cookie";
static int header_cookie_len = sizeof(header_cookie) - 1;
static const char protocol[] = "HTTP/1.1";
static int protocol_len = sizeof(protocol) - 1;
static const char delim[] = "\r\n";
static const int delim_length = 2;

static void http_evaluate_headers(struct rena *rena, client_position_t *client,
                                  struct http **cprot, int mod);

static int buffer_find(const char *buf, size_t buf_sz,
                       const char *chars, size_t chars_sz)
{
    for (int K=0; K<buf_sz; K++)
    {
        for (int M=0; M<chars_sz; M++)
        {
            if (chars[M] == buf[K])
            {
                return K;
            }
        }
    }
    return -1;
}

static int find_header(struct http *http, const char *h, int hlen)
{
    for (int i=0; http->headers && i<http->headers_used; i++)
    {
        if (http->headers[i][hlen] == ':'
            && strncasecmp(http->headers[i], h, hlen) == 0)
        {
            return i;
        }
    }
    return -1;
}

static struct http *http_create(struct rena *r, int type)
{
    struct http *ret = calloc(1, sizeof(struct http));
    ret->lookup_tree = database_instance_create(r, type);
    ret->expected_payload = -1;
    ret->total_block = sizeof(struct http);
    return ret;
}

void http_destroy(void *handler)
{
    if (handler == NULL)
        return ;

    struct http *h = (struct http *) handler;
    database_instance_destroy(&h->lookup_tree);
    // free will be not allowed on each header to be called,
    // since all pointers goes to buffer!
    free(h->headers);
    h->headers = NULL;
    free(h->headers_length);
    h->headers_length = NULL;
    free(h);
}

static void copy_internal_data(struct http *target, struct http *source,
                               size_t target_size)
{
    memmove(target, source, source->total_block);
    target->total_block = target_size;

    // lets fixing all pointers!
    for (int i=0; i<source->headers_used && source->headers; i++)
    {
        int diff_header = source->headers[i] - source->buffer;
        target->headers[i] = target->buffer + diff_header;
    }

    if (source->payload) // not nulled?
    {
        int diff_payload = source->payload - source->buffer;
        target->payload = target->buffer + diff_payload;
    }
}

static int reallocation_protocol(client_position_t *c, int olen,
                                 struct http **ptr)
{
    struct http *h = (struct http *) clients_get_protocol(c);
    const int szh = sizeof(struct http);
    size_t str_size = h->total_block - szh + MAX_STR;
    size_t new_size = h->buffer_used + olen;
    int ret = 0;

    if (new_size >= str_size)
    {
        struct http *hl = NULL;
        int r = (olen / szh) + 1;
        size_t total = h->total_block + szh * r;

        hl = calloc(1, total + 1); // one byte more to be a guardian
        copy_internal_data(hl, h, total);
        do_log(LOG_DEBUG, "done reallocation [%p (%lu) -> %p (%lu)]!",
               h, h->total_block, hl, hl->total_block);
        clients_set_protocol(c, hl);
        free(h);
        h = hl;
        ret = 1;
    }

    *ptr = h;
    return ret;
}

static int force_onto_buffer(client_position_t *c, const char *o, int olen)
{
    struct http *h = NULL;
    int ret = reallocation_protocol(c, olen, &h);
    memmove(h->buffer + h->buffer_used, o, olen);
    h->buffer_used += olen;
    /*do_log(LOG_DEBUG, "forced_on_buffer begin: [%.*s] %d", olen, o, olen);
    for (size_t si = 0; si < h->buffer_used; si++)
    {
        char ch = h->buffer[si];
        do_log(LOG_DEBUG, "fo %lu = %d (%c)", si,
            ch, ((ch <= 13)?'?':ch));
    }
    do_log(LOG_DEBUG, "force_on_buffer end: returning %d\n\n", ret);*/
    return ret;
}

static int update_buffer_forced(client_position_t *client,
                                const char *phrase, int phrase_size,
                                struct http **ptr)
{
    int rbuf = force_onto_buffer(client, phrase, phrase_size);
    if (rbuf > 0)
        *ptr = clients_get_protocol(client);
    return rbuf;
}

static int flush_pending_data_to_buffer(client_position_t *client,
                                        struct http *cprot)
{
    const char *holding = NULL;
    int holding_size = 0;
    int rbuf = -1;

    if (!client || !cprot)
        return -2;

    database_instance_get_holding(cprot->lookup_tree,
            &holding, &holding_size, 1);

    rbuf = update_buffer_forced(client, holding, holding_size, &cprot);
    if (rbuf < 0)
    {
        return -1;
    }

    return 0;
}

static int check_delimiter_header(struct http *cprot, char ch)
{
    const char *ptr = NULL;
    char repeat=0;
    //do_log(LOG_DEBUG, "call %p %u", cprot, ch);
    if (cprot->payload != NULL)
    {
        //do_log(LOG_DEBUG, "returning 0");
        return 0;
    }

    if ((ptr = strchr(delim, ch)) && *ptr != '\0')
    {
        if (ch == cprot->last_found_delimiters)
        {
            repeat = 1;
        }
        cprot->found_delimiters += 1;
        cprot->last_found_delimiters = ch;
    } else {
        cprot->found_delimiters = 0;
        cprot->last_found_delimiters = 0;
    }

    if (cprot->found_delimiters == 4
            || (repeat && cprot->found_delimiters==2))
    {
        //do_log(LOG_DEBUG, "returning 1 - %d %u",
        //       cprot->found_delimiters, repeat);
        return 1;
    }

    //do_log(LOG_DEBUG, "returning 0 - %d %u",
    //       cprot->found_delimiters, repeat);
    return 0;
}

int http_pull_reader(struct rena *rena, client_position_t *client,
                     struct http **cprot, char *buffer, size_t buffer_sz)
{
    for (int i=0; i<buffer_sz; i++)
    {
        const char *transformed = NULL;
        const char *holding = NULL;
        int transformed_size = 0;
        int holding_size = 0;
        int rbuf = 0;
        di_output_e di = database_instance_lookup(
                (*cprot)->lookup_tree, buffer[i],
                &transformed, &transformed_size);
        if (di == DBI_FEED_ME)
        {
            database_instance_add_input((*cprot)->lookup_tree, buffer[i]);
            check_delimiter_header(*cprot, 0); // no delimiter
            continue;
        }

        if (transformed)
        {
            database_instance_get_holding((*cprot)->lookup_tree,
                    &holding, &holding_size, 1);
        } else {
            database_instance_add_input((*cprot)->lookup_tree, buffer[i]);
            database_instance_get_holding((*cprot)->lookup_tree,
                    &holding, &holding_size, 0);
        }

        if (holding_size > 0)
        {
            rbuf = update_buffer_forced(client,
                    holding, holding_size, cprot);
            if (rbuf < 0)
            {
                return -1;
            }
        }

        if (transformed)
        {
            rbuf = update_buffer_forced(client,
                    transformed, transformed_size, cprot);
            if (rbuf < 0)
            {
                return -1;
            }

            database_instance_add_input((*cprot)->lookup_tree,
                    buffer[i]);
        }

        if (check_delimiter_header(*cprot, buffer[i]))
        {
            http_evaluate_headers(rena, client, cprot, 1 + i);
        }
    }

    (*cprot)->buffer_recv += buffer_sz;
    return 0;
}

int http_pull(struct rena *rena, client_position_t *client, int fd)
{
    int cfd = clients_get_fd(client);
    const char *cip = clients_get_ip(client);
    void *cssl = clients_get_ssl(client);
    struct http *cprot = NULL;
    char buffer[MAX_STR];
    size_t buffer_sz = sizeof(buffer);
    int ret = -1;
    int retry = 0;
    int is_victim = (client->type == VICTIM_TYPE);
    int first = 1;

    if (cfd != fd)
    {
        do_log(LOG_ERROR, "invalid fd %d against %d (%s)",
               fd, cfd, cip);
        return -1;
    }

    do_log(LOG_DEBUG, "fd:%d remote:%s", fd, cip);

    cprot = (struct http *) clients_get_protocol(client);
    if (cprot == NULL)
    {
        cprot = http_create(rena, is_victim);
        clients_set_protocol(client, cprot);
    }

    while (!(ret = server_read_client(
                        cfd, cssl,
                        buffer, &buffer_sz, &retry))
                && !retry)
    {
        if (http_pull_reader(rena, client, &cprot, buffer, buffer_sz) < 0)
            return -1;

        first = 0;
        buffer_sz = sizeof(buffer);
    }

    do_log(LOG_DEBUG, "fd:%d returning [%d]", cfd, ret);
    if (ret < 0) // error?
    {
        flush_pending_data_to_buffer(client, cprot);
        server_close_client(cfd, cssl, ret);
        clients_set_fd(client, -1);
        return (cprot)?0:-1;
    }
    if (first && ret > 0) // ssl annoying?
    {
        int sslwant = (ret == TT_READ) ? 1 : 0;
        clients_set_want(client, 1, sslwant);
        return ret;
    }

    clients_clear_want(client);
    //do_log(LOG_DEBUG, "buf [%.*s] (%ld) read from fd:%d",
    //       (int) cprot->buffer_used, cprot->buffer, cprot->buffer_used, cfd);
    return 0;
}

static int http_push2(struct http *pp, client_position_t *client,
                      int cfd, int pfd)
{
    size_t buffer_sz = 0;
    int res = 0;
    int retry = 0;
    void *cssl = NULL;

    if (!pp || !client)
    {
        do_log(LOG_DEBUG, "invalid data [%p] [%p]", pp, client);
        return -1;
    }

    buffer_sz = pp->buffer_used - pp->buffer_sent;

    if (clients_get_want(client) == 0 && buffer_sz == 0)
        return TT_READ; // sent all

    cssl = clients_get_ssl(client);
    /*if (client->type == VICTIM_TYPE)
        do_log(LOG_DEBUG, "sending buf [%.*s] (%lu/%lu) to fd:%d",
                (int) buffer_sz, pp->buffer + pp->buffer_sent,
                buffer_sz, pp->buffer_used, cfd); // */

    res = server_write_client(cfd, cssl,
            pp->buffer + pp->buffer_sent,
            &buffer_sz, &retry);
    if (buffer_sz == 0)
    {
        do_log(LOG_DEBUG, "ssl internal communication, fd:%d peer=%d",
               cfd, pfd);
        return (pfd < 0) ? 0 : TT_READ;
    }
    if (res < 0) // error?
    {
        server_close_client(cfd, cssl, res);
        clients_set_fd(client, -1);
        do_log(LOG_DEBUG, "writing error from fd:%d", cfd);
        return -1;
    }
    if (res > 0) // ssl annoying?
    {
        int sslwant = (res == TT_READ) ? 1 : 0;
        do_log(LOG_DEBUG, "annoyed ssl from fd:%d", cfd);
        clients_set_want(client, 0, sslwant);
        return res;
    }

    clients_clear_want(client);

    if (!retry)
    {
        pp->buffer_sent += buffer_sz;
        do_log(LOG_DEBUG, "fd:%d has been sent [%ld/%ld] bytes peer=%d",
                cfd, pp->buffer_sent, pp->buffer_used, pfd);
        if (pfd < 0 && pp->buffer_sent >= pp->buffer_used)
        {
            return 0;
        }
    }

    return (pp->buffer_sent < pp->buffer_used) ? TT_WRITE : TT_READ;
}

int http_push(struct rena *rena, client_position_t *client, int fd)
{
    client_position_t peer_raw = {NULL, INVALID_TYPE, NULL};
    client_position_t *peer = &peer_raw;
    struct http *pp = NULL;
    struct http *cc = NULL;
    int cfd = clients_get_fd(client);
    int pfd = -1;
    int is_victim = (client->type == VICTIM_TYPE);
    int ret = 0;

    if (cfd != fd)
    {
        const char *ip = clients_get_ip(client);
        do_log(LOG_ERROR, "invalid fd %d against %d (%s) to send data",
               fd, cfd, ip);
        return -1;
    }

    cc = clients_get_protocol(client);
    if (cc == NULL)
    {
        cc = http_create(rena, is_victim);
        clients_set_protocol(client, cc);
    }

    clients_get_peer(client, &peer_raw);

    if (peer->info)
    {
        clients_protocol_lock(peer, 0);
        pp = clients_get_protocol(peer);
    }

    if (pp == NULL)
    {
        do_log(LOG_DEBUG, "fd:%d hang up? Cant sent data!", cfd);
        ret = -1;
    } else {
        pfd = clients_get_fd(peer);
        ret = http_push2(pp, client, cfd, pfd);
    }

    clients_protocol_unlock(peer, 0);
    return ret;
}

static void headers_sum_1(struct http *http, const char *h, int hlen,
                          int *user)
{
    (*user)++;
}

static void headers_save(struct http *http, const char *h, int hlen,
                         int *user)
{
    do_log(LOG_DEBUG, "[%d] %.*s", *user, hlen, h);
    http->headers[*user] = h;
    http->headers_length[*user] = hlen;
    (*user)++;
}

static const char *process_headers_and_get_payload(
        struct http *http, int *hs,
        void (*fnc)(struct http *, const char *, int, int *))
{
    enum {
        FIRST_LINE_FIRST_DELIM = 0,
        FIRST_LINE_SECOND_DELIM,
        HEADER_FIRST_DELIM,
        HEADER_SECOND_DELIM,
        HEADER_THIRD_DELIM,
        HEADER_DONE
    } state = FIRST_LINE_FIRST_DELIM;
    int old_position_buf = 0;
    int position_buf = 0;
    int size_buf = http->buffer_used;
    const char *ret = NULL;

    if (http == NULL || hs == NULL || http == NULL)
    {
        abort();
        return ret;
    }

    while (ret == NULL && position_buf < size_buf)
    {
        int first_check = buffer_find(http->buffer + position_buf,
                             size_buf - position_buf,
                             delim, delim_length);

        if (first_check >= 0)
        {
            int expected_position = position_buf + first_check + 1;
            int test_line = 0;
            int next_state = -1;

            switch (state)
            {
            case FIRST_LINE_SECOND_DELIM:
                next_state = HEADER_FIRST_DELIM;
                old_position_buf = expected_position;
                // lets downgrade the protocol...
                // to cut the persistent connection!
                if (!strncmp(http->buffer
                             + old_position_buf - 2
                             - protocol_len,
                            protocol, 6))
                {
                    http->buffer[old_position_buf - 5] = '1';
                    // dot
                    http->buffer[old_position_buf - 3] = '0';
                }
                //do_log(LOG_DEBUG, "Found first line [%.*s]",
                //       old_position_buf - 2, http->buffer);
                break;
            case HEADER_SECOND_DELIM:
                // If I dont detect a continuation line...
                next_state = HEADER_FIRST_DELIM;
                test_line = buffer_find(http->buffer + expected_position, 1,
                                        "\t ", 2);
                if (test_line<0)
                {
                    fnc(http, http->buffer + old_position_buf,
                        expected_position - old_position_buf - 2, hs);
                    //do_log(LOG_DEBUG, "Found HEADER [%.*s]",
                    //       expected_position - old_position_buf - 2,
                    //       http->buffer + old_position_buf);
                    old_position_buf = expected_position;
                }

                if (buffer_find(http->buffer + expected_position, 1,
                                delim, delim_length) >= 0)
                {
                    next_state = HEADER_THIRD_DELIM;
                }
                break;
            case HEADER_DONE:
                ret = http->buffer + expected_position;
                //do_log(LOG_DEBUG, "Found end of header at %d: [%.*s]",
                //       expected_position, 10, ret);
                break;
            default:
                next_state = state + 1;
                break;
            }

            state = next_state;
        }

        position_buf += (first_check >= 0) ? first_check + 1 : 1;
    }
    return ret;
}

static char *copy_header(struct http *http, int pos)
{
    char *header = strndup(http->headers[pos], http->headers_length[pos] + 1);
    header[http->headers_length[pos]] = '\0';
    return header;
}

static int content_length_value(struct http *http, int mod)
{
    int hr=find_header(http,
            header_content_length,
            header_content_length_len);
    if (hr < 0)
    {
        return -1;
    }

    if (http->expected_payload < 0)
    {
        char *header = copy_header(http, hr);
        char *value = NULL;
        value = header + header_content_length_len + 2;
        errno = 0;
        http->expected_payload = atoi(value);
        if (errno != 0)
        {
            do_log(LOG_ERROR, "Cant convert data [%s]...", value);
            http->expected_payload = -1;
        }
        free(header);
        if (http->expected_payload < 0)
            return 0;
    }

    http->expected_payload += http->buffer_recv + mod;
    return http->expected_payload;
}

static int check_payload_length(struct http *http, int *holding_flag)
{
    int expected = 0;
    if (http->expected_payload < 0)
        return 0; // no pending!
    expected = http->buffer_recv;
    if (expected < http->expected_payload)
    {
        int hs = database_instance_get_holding_size(http->lookup_tree);
        expected += hs;
        if (hs > 0) *holding_flag = 1;
    }
    return (expected < http->expected_payload);
}

static int check_authorization(struct http *http,
                               client_position_t *client,
                               char *auth_cookie, int auth_cookie_sz)
{
    const char *cip = clients_get_ip(client);
    const char cookie_name[] = " renaproxy="; // before first can be ';' or ':'
    const int cookie_name_len = sizeof(cookie_name) - 1;
    int hr = find_header(http, header_cookie,
                         header_cookie_len);
    int check_login = 0;
    int tmp = md5_encode(cip, strnlen(cip, MAX_STR),
                         auth_cookie, &auth_cookie_sz);

    if (hr < 0)
    {
        check_login = 1;
    } else {
        char *header = copy_header(http, hr);
        char *value = strcasestr(header, cookie_name);

        if (value == NULL)
        {
            check_login = 1;
        } else {
            if (tmp != 0 || memcmp(auth_cookie,
                                   value + cookie_name_len,
                                   auth_cookie_sz))
                check_login = 1;
        }

        free(header);
    }

    return check_login; // 0 authorized, otherwhise unauthorized
}

int get_n_split_hostname(struct http *http, char **h, char **host, int *port)
{
    int hr=find_header(http, header_host, header_host_len);
    char *header = NULL;
    char *value = NULL;
    char *modified_port = NULL;
    int ret = -1;

    if (hr < 0)
    {
        return -1;
    }

    header = copy_header(http, hr);
    value = header + header_host_len + 2;
    modified_port = strchr(value, ':');

    if (modified_port)
    {
        int tmp_port;
        *modified_port = '\0';
        modified_port++;
        errno = 0;
        tmp_port = atoi(modified_port);
        if (errno != 0)
        {
            do_log(LOG_ERROR, "Cant convert data [%s]", modified_port);
            ret = 1;
        } else {
            *port = tmp_port;
            ret = 0;
        }
    } else {
        ret = 0;
    }

    *h = header;
    *host = value;
    return ret;
}

char *get_hostname_only(struct http *http)
{
    int hr=find_header(http, header_host, header_host_len);
    char *header = NULL;
    char *value = NULL;
    char *modified_port = NULL;

    if (hr < 0)
    {
        return NULL;
    }

    header = copy_header(http, hr);
    value = header + header_host_len + 2;
    modified_port = strchr(value, ':');

    if (modified_port)
    {
        *modified_port = '\0';
    } else {
        modified_port = strchr(value, '/');
        if (modified_port)
            *modified_port = '\0';
    }

    return header;
}

static int is_a_request_to_myself(struct rena *rena, const char *host,
                                  char *cookie, size_t cookie_sz)
{
    const char *suffix = database_get_suffix(rena);
    int ret = 0;
    if (!suffix)
    {
        ret = 1;
    } else {
        if (cookie != NULL)
        {
            const char domain[] = "; domain=";
            int cookie_len = strnlen(cookie, cookie_sz);
            int suffix_len = strnlen(suffix, cookie_sz);
            if (cookie_len + suffix_len + sizeof(domain) <= cookie_sz)
            {
                memcpy(cookie + cookie_len, domain, sizeof(domain) - 1);
                memcpy(cookie + cookie_len + sizeof(domain) - 1,
                       suffix, suffix_len + 1);
            } else {
                do_log(LOG_ERROR, "not enough space to save cookie! "
                                  "Turning on request to myself...");
                ret = 1;
            }
        }
        if(!strcasecmp(host, suffix + 1))
        {
            ret = 1;
        }
    }

    return ret;
}

static int verify_username_and_password(const char *user, const char *pwd)
{
    (void) user;
    (void) pwd;
    return 0; // LATER TODO: get username and passwrod from configuration?
}

static int basic_authentication(void)
{
    return 1; // TODO unimplemented!
}

static int url_authentication(void)
{
    (void) verify_username_and_password;
    return 0; // TODO unimplemented!
}

static int check_allowed_login(struct http *cprot,
                               char *location, size_t location_sz)
{
    const char delimiter[] = "?&";
    const char url_param[] = "url=";
    char *start = NULL;
    char *end = NULL;
    char *url_start = NULL;
    char *url_end = NULL;
    int len = 0;

    if (!cprot || !location)
        return 0;

    if (basic_authentication() && url_authentication())
        return 0;

    start = strcasestr(cprot->buffer, "/login"); // start of path
    if (!start)
        return 0;
    start += 6;

    end = strchr(start, ' '); // end of path
    if (!end)
        return 0;

    url_start = strcasestr(start, url_param);
    if (!url_start || url_start <= start || url_start > end
            || !strchr(delimiter, *(url_start - 1)))
    {
        return 0;
    }
    url_start += 4;

    url_end = strchr(url_start, '&');
    if (!url_end || url_end > end) url_end = end;

    len = url_end - url_start;
    //do_log(LOG_DEBUG, "redirect to (%d) %.*s", len, len, url_start);
    snprintf(location, location_sz, "%.*s", len, url_start);
    return 302;
}

static int prepare_peer_to_dispatch(struct rena *rena,
                                    client_position_t *client,
                                    client_position_t *peer,
                                    void *addresses)
{
    int vfd = server_socket_for_client(rena, addresses);
    if (vfd < 0)
    {
        return -3;
    }

    if (clients_add_peer(client, vfd) != 0
            || clients_get_peer(client, peer) != 0)
    {
        do_log(LOG_DEBUG, "problems with peer data!");
        return -3;
    }

    clients_set_userdata(client, addresses);
    clients_set_userdata(peer, addresses);
    return 0;
}

static int prepare_fake_peer(client_position_t *peer,
                             client_position_t *client,
                             struct http *cprot)
{
    if (clients_add_peer(client, -1) != 0
            || clients_get_peer(client, peer) != 0
            || cprot == NULL)
    {
        do_log(LOG_DEBUG, "problems with peer data!");
        return -3;
    }

    clients_set_protocol(peer, cprot);
    return 0;
}


static int prepare_address_from(void **addresses, char *host, int port)
{
    if (server_address_from_host(host, addresses) || *addresses == NULL)
        return 404; // LATER will send a fallback? change to 302...

    server_address_set_port(*addresses, port);
    return 0;
}

static int dispatch_new_connection(struct rena *rena,
                                   client_position_t *client,
                                   const char *value_host,
                                   void *addresses)
{
    client_position_t peer;

    if (prepare_peer_to_dispatch(rena, client, &peer, addresses))
    {
        do_log(LOG_DEBUG, "problem preparing socket tot dispatch");
        return 500;
    }

    if (value_host && server_set_client_as_secure(rena, &peer, value_host))
    {
        do_log(LOG_DEBUG, "problem setting SNI");
        return 500;
    }

    return (server_try_client_connect(rena, &peer) == -3) ? 404 : 0;
}

static void find_and_remove_header(struct http *http,
                                   const char *hdr,
                                   size_t hdr_len)
{
    int found = find_header(http, hdr, hdr_len);
    const char *hdr_found = NULL;
    const char *next = NULL;
    int diff = 0;

    if (found < 0)
    {
        return ;
    }

    hdr_found = http->headers[found];
    if (found + 1 >= http->headers_used)
    {
        next = http->payload - 2;
    } else {
        next = http->headers[found + 1];
    }

    diff = next - hdr_found;
    memmove(http->buffer + (hdr_found - http->buffer),
            next,
            http->buffer_used - (next - http->buffer));

    for (int i = found; i + 1 < http->headers_used; i++)
    {
        http->headers[i] = http->headers[i + 1] - diff;
        http->headers_length[i] = http->headers_length[i + 1];
    }
    http->headers_used -= 1;
    http->payload -= diff;
    http->buffer_used -= diff;
    if (http->buffer_used <= 0)
    {
        do_log(LOG_ERROR, "problems!");
        abort();
    }
}

static int apply_on_domain(
        struct rena *rena, struct http *http,
        int (*fnc)(struct rena *, struct http *, char *, char *, int *),
        int *u)
{
    const char *body = http->payload;
    const char *dprop = "; DOMAIN=";
    const char *terminator = ";\r\n";
    const int dprop_len = sizeof(dprop);
    int match = 0;
    char *begin = NULL;
    char *s = NULL;

    if (http->headers)
    {
        do_log(LOG_ERROR, "headers will be pointing to invalid location!!");
    }

    for (s=http->buffer; s < body; s++)
    {
        if (match > dprop_len)
        {
            if (begin == NULL)
            {
                begin = s;
            } else {
                char *end = strchr(terminator, *s);
                if (end != NULL && fnc)
                {
                    if (fnc(rena, http, begin, s, u) < 0)
                        return -1;
                    begin = NULL;
                    match = 0;
                }
            }
        } else if (match > 1) {
            int ch = toupper(*s);
            if (dprop[match] == ch)
                match++;
            else
                match = 0;
        } else {
            if (dprop[match] == *s)
                match++;
            else
                match = 0;
        }
    }

    return (s >= body) ? 1 : 0;
}

static int size_need_for_suffix(struct rena *rena, struct http *http,
                       char *begin, char *end,
                       int *userdata)
{
    const char *suffix = database_get_suffix(rena);
    const int suffix_len = strnlen(suffix, 4096);
    int len = end - begin;
    int effected = suffix_len - len;

    if (effected > 0)
        *userdata += effected;

    return 0;
}

static int copy_suffix(struct rena *rena, struct http *http,
                       char *begin, char *end, int *userdata)
{
    const char *suffix = database_get_suffix(rena);
    const int suffix_len = strnlen(suffix, 4096);
    int len = end - begin;
    int effected = suffix_len - len;

    do_log(LOG_DEBUG, "changing [%.*s] to [%.*s]",
           len, begin, suffix_len, suffix);

    memmove(begin + suffix_len, end, http->buffer_used - (end - http->buffer));
    memcpy(begin, suffix, suffix_len);

    do_log(LOG_DEBUG, "effected [%.*s]",
           ((suffix_len > len)?suffix_len:len), begin);

    http->payload += effected;
    http->buffer_used += effected;
    return 0;
}

static void adjust_domain_property(struct rena *rena,
                                   client_position_t *c,
                                   struct http **base)
{
    int new_elements = 0;

    if (apply_on_domain(rena, *base, size_need_for_suffix, &new_elements) != 1)
    {
        do_log(LOG_ERROR, "error fixing headers");
        return ;
    }

    if (new_elements > 0)
        reallocation_protocol(c, new_elements, base);

    apply_on_domain(rena, *base, copy_suffix, NULL);
}

static void adjust_expect_payload(struct http *http, int mod)
{
    if (http->expected_payload > 0)
        return ;

    int clv = content_length_value(http, mod);
    if (clv <= 0)
    {
        if (clv == http->expected_payload)
            return ;
        clv = content_length_value(http, mod);
        if (clv <= 0) return ;
    }
}

static void remove_headers(int is_victim, struct http *cprot)
{
    if (is_victim)
    {
        find_and_remove_header(cprot,
                               header_content_length,
                               header_content_length_len);
        find_and_remove_header(cprot,
                accept_ranges,
                accept_ranges_len);
        find_and_remove_header(cprot,
                header_content_md5,
                header_content_md5_len);
        return ;
    }

    // Requester: we remove connection and accept-encoding and have
    // already changed protocol from most recent to HTTP/1.0
    find_and_remove_header(cprot,
            header_connection,
            header_connection_len);
    find_and_remove_header(cprot,
            accept_encoding,
            accept_encoding_len);
}

static void check_to_disable_transformations(struct rena *rena,
                                             client_position_t *client,
                                             struct http *cprot)
{
    int do_disable = 0;
    int hr=find_header(cprot,
            header_content_type,
            header_content_type_len);
    int html = 0;

    if (hr >= 0)
    {
        char *header = copy_header(cprot, hr);
        const char *value = header + header_content_type_len + 2;
        if (config_process_header_content_type(&rena->config, value) != 1)
            do_disable = 1;
        if (strcasestr(value, "html") != NULL)
            html = 1;
        free(header);
    }

    if (hr < 0)
    {
        client_position_t peer_raw = {NULL, INVALID_TYPE, NULL};
        client_position_t *peer = &peer_raw;
        clients_get_peer(client, &peer_raw);

        if (peer->info)
        {
            struct http *pprot = clients_get_protocol(peer);
            if (pprot)
            {
                int phr=find_header(pprot,
                                    header_accept, header_accept_len);
                char *pheader = copy_header(pprot, phr);
                const char *pvalue = pheader + header_accept_len + 2;
                if (config_process_header_accept(&rena->config, pvalue) == -1)
                    do_disable = 1;
                free(pheader);
            }
        }
    }

    if (do_disable)
    {
        do_log(LOG_DEBUG, "disabling transformations on fd:%d",
               clients_get_fd(client));
        context_nothing_allowed(cprot->lookup_tree);
    } else {
        if (!html)
        {
            do_log(LOG_DEBUG, "setting full link parser");
            context_set_full_link_parser(cprot->lookup_tree);
        } else {
            do_log(LOG_DEBUG, "setting html parser");
            context_set_html_parser(cprot->lookup_tree);
        }
    }
}

static void http_evaluate_headers(struct rena *rena, client_position_t *client,
                                  struct http **cprot, int mod)
{ // client buffer locked!
    const char *payload = NULL;
    struct http *http = *cprot;

    if (http == NULL || http->payload != NULL || client == NULL)
    {
        do_log(LOG_DEBUG, "called r %p c %p h %p", rena, client, http);
        return ;
    }

    http->headers_used = 0;
    payload = process_headers_and_get_payload(http, &http->headers_used,
                                              headers_sum_1);

    if (payload == NULL)
    {
        do_log(LOG_DEBUG, "not found end of headers, error?");
        return ; // No payload, error?
    }

    http->payload = payload;

    do_log(LOG_DEBUG, "FOUND %s - %d headers and payload after %ld bytes",
           ((client->type==VICTIM_TYPE)?"VICTIM":"REQUESTER"),
           http->headers_used, payload - http->buffer);

    adjust_domain_property(rena, client, cprot);
    http = *cprot; // update
    // after this call http->payload / payload is invalid

    if (http->headers == NULL)
    {
        int n = 0;
        http->headers = malloc(sizeof(char *) * http->headers_used);
        http->headers_length = malloc(sizeof(int) * http->headers_used);
        http->payload = process_headers_and_get_payload(http, &n,
                                                         headers_save);
    }

    adjust_expect_payload(http, mod);
    remove_headers(client->type==VICTIM_TYPE, http);
    if (http->payload && http->expected_payload > 0)
    {
        int delta = http->buffer_used + http->expected_payload
                  - http->total_block - sizeof(struct http) + MAX_STR;
        if (delta > 0)
        {
            reallocation_protocol(client, delta, cprot);
            http = *cprot;
        }
    }
    check_to_disable_transformations(rena, client, http);
}

void content_length_correction(client_position_t *c, struct http **base)
{
    char nsz[16];
    struct http *cprot = *base;
    int hcl = find_header(cprot,
                          header_content_length, header_content_length_len);
    char *header = NULL;
    char *value = NULL;
    int clv = -1;
    int dclv = -1;
    int nclv = -1;
    int ndclv = -1;

    if (hcl < 0)
    {
        return ; // No header to re-calculate!
    }

    header = copy_header(cprot, hcl);
    value = header + header_content_length_len + 2;
    clv = atoi(value);
    free(header);
    header = NULL;

    nclv = cprot->buffer + cprot->buffer_used - cprot->payload;
    if (clv == nclv)
    {
        return ;
    }

    dclv = (int)(log10(clv) + 1); // clv set to value; dclv set to value digits
    ndclv = (int)(log10(nclv) + 1);

    char *value_header = ((char *)cprot->headers[hcl])
                       + header_content_length_len + 2;
    int shift = ndclv - dclv;

    if (ndclv > dclv)
    {
        // 1. allocation
        reallocation_protocol(c, shift, base);
        cprot = *base;
        value_header = ((char *)cprot->headers[hcl])
                     + header_content_length_len + 2;
        // 2. fixing buffer : move to right
        memmove(value_header + shift, value_header,
                cprot->buffer_used - (value_header - cprot->buffer));
    } else if (ndclv < dclv)
    {
        // 1. allocation : not needed
        // 2. fixing buffer : move to left
        memmove(value_header, value_header - shift,
                cprot->buffer_used - (value_header - cprot->buffer + shift));
    }

    if (shift != 0)
    {
        // 3. fixing length
        cprot->headers_length[hcl] += shift;
        // 4. fixing pointers from hcl + 1 to ending
        for (int K=hcl + 1; K < cprot->headers_used; K++)
            cprot->headers[K] += shift;
    }

    int ed = snprintf(nsz, sizeof(nsz), "%d", nclv);
    if (ed >= sizeof(nsz))
    {
        do_log(LOG_ERROR, "Truncation [%d] during convertion to string",
               nclv);
    }

    memmove(value_header, nsz, ndclv);
}

static int handle_request_of_connection(struct rena *rena,
                                        client_position_t *client,
                                        struct http *cprot)
{
    char location_uri[MAX_STR] = { 0, };
    char authorization_cookie[MAX_STR] = { 0, };
    char *header_host = NULL;
    char *value_host = NULL;
    void *is_ssl = clients_get_ssl(client);
    int port_host = (!is_ssl) ? DEFAULT_HTTP_PORT : DEFAULT_HTTPS_PORT;
    int error_code = 0;
    int loopback_host = 0;
    int authorization = 0;
    void *addresses = NULL;

    if (get_n_split_hostname(cprot, &header_host, &value_host, &port_host) < 0)
    {
        do_log(LOG_DEBUG, "header host not found");
        error_code = 404;
        goto fake_conn;
    }

    do_log(LOG_DEBUG, "accessing host[%s] [dropped protocol/port/query/path]",
           value_host);

    authorization = check_authorization(cprot, client,
                                        authorization_cookie,
                                        sizeof(authorization_cookie));
    loopback_host = is_a_request_to_myself(rena, value_host,
                                           authorization_cookie,
                                           sizeof(authorization_cookie));

    if (loopback_host)
    {
        error_code = check_allowed_login(cprot,
                                         location_uri,
                                         sizeof(location_uri));
        if (error_code != 0)
        {
            goto fake_conn;
        }

        if (!authorization)
        {
            error_code = 401; // unauthorized
            goto fake_conn;
        }
    }

    if (authorization)
    {
        error_code = 403; // forbidden
        goto fake_conn;
    }

    if ((error_code = prepare_address_from(&addresses,
                                           value_host, port_host)))
    {
        do_log(LOG_DEBUG, "cant resolve address");
        // LATER sending a fallback change error_code to 302 and set url
        goto fake_conn;
    }

    content_length_correction(client, &cprot);

    error_code = dispatch_new_connection(rena, client,
            (is_ssl) ? value_host : NULL,
            addresses);

fake_conn:
    free(header_host);

    if (error_code)
    {
        return -1;
    }

    return TT_READ;
}

int http_evaluate(struct rena *rena, client_position_t *client)
{
    struct http *cprot = (struct http *) clients_get_protocol(client);
    int flush_holding = 0;
    int pret = -1;

    if (cprot->payload == NULL)
    {
        return TT_READ; // No payload? Keep reading!
    }

    pret = check_payload_length(cprot, &flush_holding);

    { // TODO not sure about the last piece behavior!
        client_position_t peer_raw = {NULL, INVALID_TYPE, NULL};
        client_position_t *peer = &peer_raw;
        clients_get_peer(client, &peer_raw);

        do_log(LOG_DEBUG, "fd:%d peer=%d payload_check=%d flush=%d",
            clients_get_fd(client),
            ((peer->info==NULL)?-1:clients_get_fd(peer)),
            pret, flush_holding);
    }

    if (!pret)
    {
        if (flush_holding) // last piece pending?
        {
            if (flush_pending_data_to_buffer(client, cprot) < 0)
            {
                return -1;
            }
        }
    }

    if (clients_get_fd(client) < 0)
    {
        do_log(LOG_DEBUG, "client dropped connection?");
        return -1;
    }

    if (client->type == REQUESTER_TYPE)
    {
        if (pret)
        {
            do_log(LOG_DEBUG, "going to sleep for more data");
            return TT_READ;
        }

        return handle_request_of_connection(rena, client, cprot);
    }

    return TT_READ;
}

int http_sent_done(void *protocol)
{
    struct http *p = (struct http *) protocol;
    if (p == NULL) return 1;
    if (p->buffer_sent >= p->buffer_used) return 1;
    return 0;
}
