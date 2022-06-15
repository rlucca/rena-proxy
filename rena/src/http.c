#define _GNU_SOURCE
#include "global.h"
#include "http.h"
#include "database.h"
#include "context_full_link.h"
#include "context_html.h"
#include "task_manager.h"
#include "server.h"
#include "md5.h"
#include "template.h"

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

static text_t header_sts             = { 25, "Strict-Transport-Security" };
static text_t header_accept_encoding = { 15, "Accept-Encoding" };
static text_t header_content_length  = { 14, "Content-Length" };
static text_t header_authentication  = { 14, "Authentication" };
static text_t header_accept_ranges   = { 13, "Accept-Ranges" };
static text_t header_content_type    = { 12, "Content-Type" };
static text_t header_content_md5     = { 11, "Content-Md5" };
static text_t header_connection      = { 10, "Connection" };
static text_t protocol               = {  8, "HTTP/1.1" };
static text_t header_accept          = {  6, "Accept" };
static text_t header_cookie          = {  6, "Cookie" };
static text_t header_origin          = {  6, "Origin" };
static text_t header_host            = {  4, "Host" };
static text_t delim_break            = {  2, "\r\n" };
static text_t delim_continue         = {  2, "\t " };

static void http_evaluate_headers(struct rena *rena, client_position_t *client,
                                  struct http **cprot, int mod);


static int buffer_find(const char *buf, size_t buf_sz, text_t *t)
{
    for (int K=0; K<buf_sz; K++)
    {
        for (int M=0; M<t->size; M++)
        {
            if (t->text[M] == buf[K])
            {
                return K;
            }
        }
    }
    return -1;
}

static int find_header2(struct http *http,
                        const char *name, int name_len)
{
    for (int i=0; http->headers && i<http->headers_used; i++)
    {
        if (http->headers[i][name_len] == ':'
            && strncasecmp(http->headers[i], name, name_len) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int find_header(struct http *http, text_t *h)
{
    return find_header2(http, h->text, h->size);
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

    if ((ptr = strchr(delim_break.text, ch)) && *ptr != '\0')
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
                     struct http **cprot, text_t *buffer)
{
    for (int i=0; i<buffer->size; i++)
    {
        const char *transformed = NULL;
        const char *holding = NULL;
        int transformed_size = 0;
        int holding_size = 0;
        int rbuf = 0;
        di_output_e di = database_instance_lookup(
                (*cprot)->lookup_tree, buffer->text[i],
                &transformed, &transformed_size);
        if (di == DBI_FEED_ME)
        {
            database_instance_add_input((*cprot)->lookup_tree,
                                        buffer->text[i]);
            check_delimiter_header(*cprot, 0); // no delimiter
            continue;
        }

        if (transformed)
        {
            database_instance_get_holding((*cprot)->lookup_tree,
                    &holding, &holding_size, 1);
        } else {
            database_instance_add_input((*cprot)->lookup_tree,
                                        buffer->text[i]);
            database_instance_get_holding((*cprot)->lookup_tree,
                    &holding, &holding_size, 0);
        }

        if (holding_size > 0)
        {
            rbuf = update_buffer_forced(client, holding, holding_size, cprot);
            if (rbuf < 0)
            {
                return -1;
            }
        }

        if (transformed)
        {
            rbuf = update_buffer_forced(client, transformed, transformed_size,
                                        cprot);
            if (rbuf < 0)
            {
                return -1;
            }

            database_instance_add_input((*cprot)->lookup_tree,
                                        buffer->text[i]);
        }

        if (check_delimiter_header(*cprot, buffer->text[i]))
        {
            http_evaluate_headers(rena, client, cprot, 1 + i);
        }
    }

    (*cprot)->buffer_recv += buffer->size;
    return 0;
}

int http_pull(struct rena *rena, client_position_t *client, int fd)
{
    int cfd = clients_get_fd(client);
    const char *cip = clients_get_ip(client);
    void *cssl = clients_get_ssl(client);
    struct http *cprot = NULL;
    text_t buffer;
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

    while (!(ret = server_read_client(cfd, cssl, &buffer, &retry)) && !retry)
    {
        if (http_pull_reader(rena, client, &cprot, &buffer) < 0)
            return -1;

        first = 0;
    }

    do_log(LOG_DEBUG, "fd:%d returning [%d]", cfd, ret);
    if (ret < 0) // error?
    {
        flush_pending_data_to_buffer(client, cprot);
        //server_close_client(cfd, cssl, ret);
        //clients_set_fd(client, -1);
        //return (cprot)?0:-1;
        return -1;
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

    res = server_write_client(cfd, cssl, pp->buffer + pp->buffer_sent,
                              &buffer_sz, &retry);
    if (buffer_sz == 0)
    {
        do_log(LOG_DEBUG, "ssl internal communication, fd:%d peer=%d",
               cfd, pfd);
        return (pfd < 0) ? 0 : TT_READ;
    }
    if (res < 0) // error?
    {
        //server_close_client(cfd, cssl, res);
        //clients_set_fd(client, -1);
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
                             size_buf - position_buf, &delim_break);

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
                             - protocol.size,
                            protocol.text, 6))
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
                                        &delim_continue);
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
                                &delim_break) >= 0)
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
    int hr=find_header(http, &header_content_length);
    if (hr < 0)
    {
        return -1;
    }

    if (http->expected_payload < 0)
    {
        char *header = copy_header(http, hr);
        char *value = NULL;
        value = header + header_content_length.size + 2;
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

static void remove_http_header(struct http *http, int found)
{
    const char *hdr_found = NULL;
    const char *next = NULL;
    int diff = 0;

    hdr_found = http->headers[found];
    if (found + 1 >= http->headers_used)
    {
        next = http->payload - 2;
    } else {
        next = http->headers[found + 1];
    }

    diff = next - hdr_found;
    memmove((char *) hdr_found,  (char *) next,
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

static int extract_substring_location_of_cookie(int (*out)[2],
                                      struct http *cprot, text_t *name,
                                      int found)
{
    // LATER appears that this function and apply on domain are very close...
    int match = -1;
    int first = 1;
    int ret = 1;
    const char *s, *base, *end;
    if (found > cprot->headers_used)
        return 0;
    base = cprot->headers[found];
    end = base + cprot->headers_length[found];
    for (s = base; s < end; s++)
    {
        if (match + 1 > name->size)
        {
            if (**out == -1)
            {
                **out = s - base - name->size - 1;
            } else {
                if (';' == *s)
                {
                    out[0][1] = s - base;
                    return 0;
                }
            }
        } else if (match >= 0) {
            int ch = tolower(*s);
            if (name->text[match] == ch)
                match++;
            else
                match = -1;
        } else {
            if ((first && ':' == *s) || ';' == *s)
            {
                match++;
                if (!first) ret = 0;
                first = 0;
            }
        }
    }

    if (**out >= 0)
        out[0][1] = s - base;
    else
        ret = 0;

    return ret;
}

static void remove_substring_cookie(struct http *cprot, int only, int found,
                              int (*loc)[2])
{
    if (!loc || loc[0][1] <= loc[0][0])
        return ; // not found!

    if (only) // remove all header iff it is the only element
    {
        remove_http_header(cprot, found);
        return ;
    }

    int diff = loc[0][1] - loc[0][0];
    char *base = (char *) cprot->headers[found];
    if (loc[0][0] == 6) // why 6? strlen("cookie")
    {
        loc[0][0]++;
        loc[0][1]++;
    }
    memmove(base + loc[0][0], base + loc[0][1],
            cprot->buffer_used - loc[0][1] - (base - cprot->buffer));
    cprot->headers_length[found] -= diff;

    for (int i = found + 1; i < cprot->headers_used; i++)
    {
        cprot->headers[i] -= diff;
    }

    cprot->payload -= diff;
    cprot->buffer_used -= diff;
    if (cprot->buffer_used <= 0)
    {
        do_log(LOG_ERROR, "problems!");
        abort();
    }
}

static int check_authorization(struct http *http,
                               client_position_t *client, text_t *tcookie)
{
    const char *cip = clients_get_ip(client);
    // before cookiename, first can be ';' or ':'
    static text_t cookie_name = { 11, " renaproxy=" };
    int hr = find_header(http, &header_cookie);
    int check_login = 0;
    int tmp = md5_encode(cip, strnlen(cip, MAX_STR), tcookie);

    if (hr < 0)
    {
        check_login = 1;
    } else {
        int loc[2] = { -1, -1 };
        int only = extract_substring_location_of_cookie(&loc, http,
                                                        &cookie_name, hr);
        if (loc[1] < 0 || loc[1] <= loc[0]) // not found!
        {
            check_login = 1;
        } else {
            if (tmp != 0 // problem generating...
                    || memcmp(tcookie->text,
                           http->headers[hr] + loc[0] + cookie_name.size + 1,
                           tcookie->size)) // or is not equal to generated!
            {
                check_login = 1;
            }

            remove_substring_cookie(http, only, hr, &loc);
        }
    }

    if (check_login && find_header(http, &header_origin) >= 0)
        check_login = 0;

    return check_login; // 0 authorized, otherwhise unauthorized
}

int get_n_split_hostname(struct http *http, char **h, char **host, int *port)
{
    int hr=find_header(http, &header_host);
    char *header = NULL;
    char *value = NULL;
    char *modified_port = NULL;
    int ret = -1;

    if (hr < 0)
    {
        return -1;
    }

    header = copy_header(http, hr);
    value = header + header_host.size + 2;
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

static int is_a_request_to_myself(struct rena *rena, const char *host,
                                  text_t *tcookie)
{
    const char *suffix = NULL;
    int ret = 0;
    if (host == NULL || *host == '\0')
        return 1;

    suffix = database_get_suffix(rena);
    if (!suffix)
    {
        ret = 1;
    } else {
        if (tcookie != NULL)
        {
            const char domain[] = "; domain=";
            int suffix_len = strnlen(suffix, sizeof(tcookie->text));
            int length = tcookie->size + suffix_len + sizeof(domain);
            if (length <= sizeof(tcookie->text))
            {
                char *end_cookie = tcookie->text + tcookie->size;
                int domain_len = sizeof(domain) - 1;
                memcpy(end_cookie, domain, domain_len);
                memcpy(end_cookie + domain_len, suffix, suffix_len + 1);
                tcookie->size += suffix_len + domain_len;
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

static int basic_authentication(struct http *http)
{
    int hr=find_header(http, &header_authentication);
    if (hr >= 0)
    {
        do_log(LOG_ERROR, "http/1.1 authentication is not implemented!");
    }
    return 1;
}

static int verify_path_to_authenticate(struct http *http, char *paths[2])
{
    const char allowed_path[] = "/login";

    paths[0] = strcasestr(http->buffer, allowed_path);
    if (!paths[0])
        return 1;
    paths[0] += 6;

    if (*paths[0] == '?')
        paths[0] += 1;

    paths[1] = strchr(paths[0], ' ');
    if (!paths[1] || (paths[0] + 1) == paths[1])
        return 1;

    return 0;
}

static int extract_position_of_param(char *out[2],
                                     const char *param,
                                     const int param_len,
                                     char *path[2])
{
#define DELIM '&'
    out[0] = strcasestr(path[0], param);
    if (!out[0] || out[0] > path[1])
    {
        return 1;
    }
    if (out[0] != path[0] && *(out[0] - 1) != DELIM)
    {
        return 1;
    }
    out[0] += param_len;

    out[1] = strchr(out[0], DELIM);
    if (!out[1] || out[1] > path[1]) out[1] = path[1];

    return 0;
#undef DELIM
}

static int path_authentication(struct rena *rena, char *paths[2])
{
    char *user[2] = { NULL, NULL };
    char *pass[2] = { NULL, NULL };
    text_t password;

    if (extract_position_of_param(user, "user=", 5, paths))
        return 1;
    if (extract_position_of_param(pass, "pass=", 5, paths))
        return 1;
    if (md5_encode(pass[0], pass[1] - pass[0], &password) != 0)
        return 1;

    const char *up[2] = { user[0], password.text };
    size_t up_len[2] = { user[1] - user[0], password.size };
    return database_verify_userlist(rena, up, up_len);
}

static int extract_location_from(text_t *tlocation, char *path[2])
{
    const char param[] = "url=";
    const int param_len = sizeof(param) - 1;
    char *url_location[2] = { NULL, NULL };

    if (extract_position_of_param(url_location, param, param_len, path))
        return 1;

    int len = url_location[1] - url_location[0];
    if (len + 1 > tlocation->size)
        return 1;

    do_log(LOG_DEBUG, "redirect to (%d) %.*s", len, len, url_location[0]);
    tlocation->size = snprintf(tlocation->text, tlocation->size,
                               "%.*s", len, url_location[0]);
    return 0;
}

static int check_allowed_login(struct rena *rena, struct http *cprot,
                               int authorization, text_t *tlocation)
{
    char *paths[2] = { NULL, NULL };

    if (!cprot || !tlocation)
        return 0;

    if (verify_path_to_authenticate(cprot, paths))
        return 0;

    if (authorization) // if not authorized already...
    {
        if (basic_authentication(cprot) && path_authentication(rena, paths))
            return 0;
    }

    if (extract_location_from(tlocation, paths))
        return 0;

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
    int cap = clients_add_peer(client, -1);
    if ((cap != 0 && cap != -3)
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

static void find_and_remove_header(struct http *http, text_t *hdr)
{
    int found = find_header(http, hdr);

    if (found < 0)
    {
        return ;
    }

    remove_http_header(http, found);
}

static void find_hsts_and_remove_includeSubDomains(struct http *http)
{
    const char property[] = " includeSubDomains";
    int found = find_header(http, &header_sts);
    const char *hdr_found = NULL;
    const char *prp_found = NULL;
    const char *prp_end = NULL;
    int diff = 0;

    if (found < 0)
    {
        return ;
    }

    hdr_found = http->headers[found];
    prp_found = strcasestr(http->headers[found], property);
    if (!prp_found || prp_found >= http->headers_length[found] + hdr_found)
    {
        return ; // not do anything!
    }

    prp_end = prp_found + sizeof(property) - 1;
    if (*(prp_found - 1) == ';')
        prp_found--;
    else
        if (*(prp_end + 1) == ';')
            prp_end++;

    diff = prp_end - prp_found;
    memmove(http->buffer + (prp_found - http->buffer),
            prp_end,
            http->buffer_used - (prp_end - http->buffer));

    for (int i = found; i < http->headers_used; i++)
    {
        if (i != found)
            http->headers[i] = http->headers[i] - diff;
        else
            http->headers_length[i] = http->headers_length[i] - diff;
    }
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
    static text_t dprop = { 8, "; DOMAIN=" };
    const char *body = http->payload;
    const char *terminator = ";\r\n";
    int match = 0;
    char *begin = NULL;
    char *s = NULL;

    if (http->headers)
    {
        do_log(LOG_ERROR, "headers will be pointing to invalid location!!");
    }

    for (s=http->buffer; s < body; s++)
    {
        if (match > dprop.size)
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
        } else {
            int ch = toupper(*s);
            if (dprop.text[match] == ch)
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
        find_and_remove_header(cprot, &header_content_length);
        find_and_remove_header(cprot, &header_accept_ranges);
        find_and_remove_header(cprot, &header_content_md5);
        find_hsts_and_remove_includeSubDomains(cprot);
        return ;
    }

    // Requester: we remove connection and accept-encoding and have
    // already changed protocol from most recent to HTTP/1.0
    find_and_remove_header(cprot, &header_connection);
    find_and_remove_header(cprot, &header_accept_encoding);
}

static void check_to_disable_transformations(struct rena *rena,
                                             client_position_t *client,
                                             struct http *cprot)
{
    int do_disable = 0;
    int hr=find_header(cprot, &header_content_type);
    int html = 0;

    if (hr >= 0)
    {
        char *header = copy_header(cprot, hr);
        const char *value = header + header_content_type.size + 2;
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
                int phr=find_header(pprot, &header_accept);
                char *pheader = copy_header(pprot, phr);
                const char *pvalue = pheader + header_accept.size + 2;
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
    int hcl = find_header(cprot, &header_content_length);
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
    value = header + header_content_length.size + 2;
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
                       + header_content_length.size + 2;
    int shift = ndclv - dclv;

    if (ndclv > dclv)
    {
        // 1. allocation
        reallocation_protocol(c, shift, base);
        cprot = *base;
        value_header = ((char *)cprot->headers[hcl])
                     + header_content_length.size + 2;
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

static int fill_fake_connection_buffer(text_t *out,
                                       text_t *auth, text_t *uri,
                                       int error_code)
{
    if (error_code == 302)
    {
        const char *auth_cookie_ptr = auth->text;
        const char *location_ptr = uri->text;
        if (*location_ptr == '\0') location_ptr = NULL;
        if (*auth_cookie_ptr == '\0') auth_cookie_ptr = NULL;
        int w = generate_redirect_to(out, auth_cookie_ptr, location_ptr);
        if (w <= 0)
        {
            return -1;
        }

    } else {
        int w = generate_error(out, error_code);
        if (w <= 0)
        {
            return -1;
        }
    }

    return 0;
}

static int dispatch_fake_connection(struct rena *rena,
                                    client_position_t *client,
                                    text_t *buf)
{
    struct http *fake = http_create(rena, 1);
    client_position_t dummy;

    if (prepare_fake_peer(&dummy, client, fake))
    {
        do_log(LOG_DEBUG, "error creating dummy peer of client");
        return -1;
    }

    if (http_pull_reader(rena, &dummy, &fake, buf) < 0)
        return -1;

    return 0;
}

static int handle_request_of_connection(struct rena *rena,
                                        client_position_t *client,
                                        struct http *cprot)
{
    text_t location_uri = { 0, };
    text_t authorization_cookie = { 0, };
    char *header_host = NULL;
    char *value_host = NULL;
    void *is_ssl = clients_get_ssl(client);
    int port_host = (!is_ssl) ? DEFAULT_HTTP_PORT : DEFAULT_HTTPS_PORT;
    int error_code = 0;
    int loopback_host = 0;
    int authorization = 0;
    void *addresses = NULL;

    location_uri.size = MAX_STR;
    authorization_cookie.size = MAX_STR;

    if (get_n_split_hostname(cprot, &header_host, &value_host, &port_host) < 0)
    {
        do_log(LOG_DEBUG, "header host not found");
        error_code = 404;
        goto fake_conn;
    }

    do_log(LOG_DEBUG, "accessing host[%s] [dropped protocol/port/query/path]",
           value_host);

    authorization = check_authorization(cprot, client, &authorization_cookie);
    loopback_host = is_a_request_to_myself(rena, value_host,
                                           &authorization_cookie);

    if (loopback_host)
    {
        error_code = check_allowed_login(rena, cprot, authorization,
                                         &location_uri);
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
        // LATER sending a fallback change inside prepare_address_from
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
        text_t ans;
        if (fill_fake_connection_buffer(&ans, &authorization_cookie,
                                        &location_uri, error_code) < 0)
        {
            return -1;
        }

        if (dispatch_fake_connection(rena, client, &ans) < 0)
        {
            return -1;
        }

        return TT_WRITE;
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
        if (client->type == REQUESTER_TYPE)
            do_log(LOG_DEBUG, "client dropped connection?");
        else
            do_log(LOG_DEBUG, "victim dropped connection?");
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

int http_sent_done(void *cprot)
{
    struct http *p = (struct http *) cprot;
    if (p == NULL) return 1;
    if (p->buffer_sent >= p->buffer_used) return 1;
    return 0;
}

int http_bytes_sent(void *cprot, char *out, int out_sz)
{
    struct http *p = (struct http *) cprot;
    int w = 0;
    if (p != NULL)
    {
        w = p->buffer_sent;
    }

    int ret = snprintf(out, out_sz, "%d", w);
    return (ret > out_sz) ? -1 : ret;
}

int http_find_header(void *cprot, const char *name, int name_len)
{
    struct http *p = (struct http *) cprot;
    if (!p || !name || name_len <= 0) return -1;
    return find_header2(p, name, name_len);
}

int http_header_value(void *cprot, char *out, int out_sz, int pos)
{
    struct http *p = (struct http *) cprot;
    if (!p || !out || out_sz <= 0 || pos < 0) return -1;
    int length = p->headers_length[pos];
    if (out_sz < length) return -1;
    memcpy(out, p->headers[pos], length);
    return length;
}
