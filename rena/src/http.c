
#include "global.h"
#include "http.h"
#include "database.h"
#include "task_manager.h"
#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct http {
    struct database_object *lookup_tree;
    int headers_used;
    const char **headers;
    int *headers_length;
    int expected_payload;
    const char *payload;

    size_t total_block;
    size_t buffer_sent;
    size_t buffer_used;
    char buffer[MAX_STR]; // at_least MAX_STR!! CAUTION: not finished in zero!
};

static const char header_content_length[] = "Content-Length";
static int header_content_length_len = sizeof(header_content_length) - 1;
static const char header_host[] = "Host";
static int header_host_len = sizeof(header_host) - 1;
static const char header_connection[] = "Connection";
static int header_connection_len = sizeof(header_connection) - 1;
static const char protocol[] = "HTTP/1.1";
static int protocol_len = sizeof(protocol) - 1;

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

static int force_onto_buffer(client_position_t *c, const char *o, int olen)
{
    struct http *h = (struct http *) clients_get_protocol(c);
    size_t old_size = h->total_block;
    size_t new_size = h->buffer_used + olen;
    int ret = 0;
    if (h->payload != NULL && h->expected_payload > 0)
    {
        int preview = h->expected_payload + (h->payload - h->buffer);
        new_size = (new_size > preview) ? new_size : preview;
    }
    if (new_size > MAX_STR && new_size > h->total_block)
    {
        size_t new_size_data = new_size + sizeof(struct http);
        struct http *hl = calloc(1, new_size_data);
        if (hl != NULL)
        {
            int diff_payload = h->payload - h->buffer;
            if (old_size == 0)
                old_size = sizeof(struct http);
            memmove(hl, h, old_size);

            for (int i=0; i<h->headers_used && h->headers; i++)
            {
                int diff_header = h->headers[i] - h->buffer;
                hl->headers[i] = hl->buffer + diff_header;
                hl->headers_length[i] = hl->headers_length[i];
            }

            if (h->payload) // not nulled?
                hl->payload = hl->buffer + diff_payload;

            hl->total_block = new_size_data;
            clients_set_protocol(c, hl);
            ret = 1;
            do_log(LOG_DEBUG, "from %p to %p (%lu expect %d)", h, hl, new_size_data, h->expected_payload);
            free(h);
            h = hl;
        } else {
            do_log(LOG_ERROR, "oh nooo! realloc failed!");
            abort();
            return -1;
        }
    }

    if (h->total_block > 0)
    {
        size_t sz = sizeof(struct http) - MAX_STR;
        size_t pr = h->total_block;
        size_t ac = h->buffer_used + olen;
        if (pr <= ac)
        {
            do_log(LOG_DEBUG, "oh nooo!! raw [%lu] writing [%lu] struct %lu",
                   pr, ac, sz);
        }
    }

    memcpy(h->buffer + h->buffer_used, o, olen);
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

int http_pull(struct rena *rena, client_position_t *client, int fd)
{
    int cfd = clients_get_fd(client);
    const char *cip = clients_get_ip(client);
    void *cssl = clients_get_ssl(client);
    struct http *cprot = (struct http *) clients_get_protocol(client);
    char buffer[MAX_STR];
    size_t buffer_sz = MAX_STR;
    int ret = -1;
    int retry = 0;
    int is_victim = (client->type == VICTIM_TYPE);

    if (cfd != fd)
    {
        do_log(LOG_ERROR, "invalid client fd %d against %d (%s)",
               fd, cfd, cip);
        return -1;
    }

    if (cprot == NULL)
    {
        cprot = http_create(rena, is_victim);
        clients_set_protocol(client, cprot);
    }

    ret = server_read_client(cfd, cssl, buffer, &buffer_sz, &retry);
    if (ret < 0) return -1; // error?
    if (ret > 0) return ret; // ssl annoying?

    for (int i=0; !retry && i<buffer_sz; i++)
    {
        const char *transformed = NULL;
        const char *holding = NULL;
        int transformed_size = 0;
        int holding_size = 0;
        int rbuf = 0;
        di_output_e di = database_instance_lookup(
                cprot->lookup_tree, buffer[i],
                &transformed, &transformed_size);
        if (di == DBI_FEED_ME)
        {
            database_instance_add_input(cprot->lookup_tree, buffer[i]);
            continue;
        }

        if (transformed)
        {
            database_instance_get_holding(cprot->lookup_tree,
                    &holding, &holding_size, 1);
        } else {
            database_instance_add_input(cprot->lookup_tree, buffer[i]);
            database_instance_get_holding(cprot->lookup_tree,
                    &holding, &holding_size, 0);
        }

        if (holding_size > 0)
        {
            rbuf = update_buffer_forced(client,
                    holding, holding_size, &cprot);
            if (rbuf < 0)
                return -1;
        }

        if (transformed)
        {
            rbuf = update_buffer_forced(client,
                    transformed, transformed_size, &cprot);
            if (rbuf < 0)
                return -1;

            database_instance_add_input(cprot->lookup_tree,
                    buffer[i]);
        }
    }

    return 0;
}

int http_push(struct rena *rena, client_position_t *client, int fd)
{
    client_position_t peer_raw = {NULL, INVALID_TYPE, NULL};
    client_position_t *peer = &peer_raw;
    struct http *pp = NULL;
    struct http *cc = NULL;
    int cfd = clients_get_fd(client);
    int is_victim = (client->type == VICTIM_TYPE);

    if (cfd != fd)
    {
        const char *ip = clients_get_ip(client);
        do_log(LOG_ERROR, "invalid client fd %d against %d (%s)",
               fd, cfd, ip);
        return -1;
    }

    clients_get_peer(client, &peer_raw);

    if (peer->info)
    {
        pp = clients_get_protocol(peer);
    }

    if (pp == NULL)
    {
        do_log(LOG_DEBUG, "client hang up?");
        return -1;
    }

    if (cc == NULL)
    {
        cc = http_create(rena, is_victim);
        clients_set_protocol(client, cc);
    } else {
        cc = clients_get_protocol(client);
        if (cc == NULL)
        {
            do_log(LOG_ERROR, "client hang up!");
            return -1;
        }
    }

    if (pp->buffer_sent < pp->buffer_used)
    {
        void *cssl = clients_get_ssl(client);
        size_t buffer_sz = pp->buffer_used - pp->buffer_sent;
        int retry = 0;
        if (is_victim)
            do_log(LOG_DEBUG, "sending buf [%.*s] (%lu)",
                   (int) buffer_sz, pp->buffer + pp->buffer_sent,
                   buffer_sz);
        int ret = server_write_client(cfd, cssl,
                        pp->buffer + pp->buffer_sent,
                        &buffer_sz, &retry);
        if (ret < 0) return -1; // error?
        if (ret > 0) return ret; // ssl annoying?
        if (!retry)
        {
            pp->buffer_sent += buffer_sz;
            do_log(LOG_DEBUG, "fd:%d has been sent [%ld/%ld] bytes",
                    fd, pp->buffer_sent, pp->buffer_used);
        }
        if (pp->buffer_sent < pp->buffer_used)
            return TT_WRITE;
    }

    return TT_READ;
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
    static const char delim[] = "\r\n";
    static const int delim_length = 2;
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

    do_log(LOG_DEBUG, "buf [%.*s] (%d)", size_buf, http->buffer, size_buf);
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

static int check_payload_length(struct http *http)
{
    int hr=find_header(http,
            header_content_length,
            header_content_length_len);
    int payload = -1;
    if (hr < 0)
    {
        return 0;
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
        {
            return 1;
        }
    }

    payload = http->buffer_used - (http->payload - http->buffer);
    return (payload < http->expected_payload);
}

static int check_authorization(client_position_t *client)
{
    return 0;
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
        *modified_port = '\0';
        modified_port++;
        errno = 0;
        *port = atoi(modified_port);
        if (errno != 0)
        {
            do_log(LOG_ERROR, "Cant convert data [%s]", modified_port);
            *port = -1;
            ret = 1;
        } else {
            ret = 0;
        }
    } else {
        ret = 0;
    }

    *h = header;
    *host = value;
    return ret;
}

static void *recover_address_from_hostname(struct http *http,
                                           client_position_t *client,
                                           int *port)
{
    char *header = NULL;
    char *value = NULL;
    void *addresses = NULL;
    int hr=get_n_split_hostname(http, &header, &value, port);

    if (hr < 0)
    {
        do_log(LOG_DEBUG, "header host not found");
        return NULL;
    }

    do_log(LOG_DEBUG, "header host found [%s]", value);
    server_address_from_host(value, &addresses);
    free(header);

    return addresses;
}

static int dispatch_new_connection(struct rena *rena,
                                   client_position_t *client,
                                   struct http *http)
{
    client_position_t peer;
    int port = -1;
    void *is_ssl = NULL;
    void *addresses = recover_address_from_hostname(
                            http, client, &port);
    int vfd=-1;

    if (addresses == NULL)
    {
        return -1;
    }

    clients_set_userdata(client, addresses);
    is_ssl = clients_get_ssl(client);

    if (port < 0)
    {
        port = (!is_ssl) ? DEFAULT_HTTP_PORT : DEFAULT_HTTPS_PORT;
    }

    server_address_set_port(addresses, port);

    vfd = server_socket_for_client(rena, addresses);
    if (vfd < 0)
    {
        return -3;
    }

    if (clients_add_peer(client, vfd) != 0
            || clients_get_peer(client, &peer) != 0)
    {
        do_log(LOG_DEBUG, "problems with peer data!");
        return -3;
    }

    if (is_ssl && server_set_client_as_secure(rena, &peer) != 0)
    {
        do_log(LOG_DEBUG, "problems creating ssl data!");
        return -3;
    }

    clients_set_userdata(&peer, addresses);
    return server_try_client_connect(rena, &peer);
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

int http_evaluate(struct rena *rena, client_position_t *client)
{
    struct http *cprot = (struct http *) clients_get_protocol(client);
    int pret = -1;

    if (cprot->payload == NULL)
    {
        cprot->headers_used = 0;
        const char *payload = process_headers_and_get_payload(
                cprot, &cprot->headers_used,
                headers_sum_1);

        if (payload == NULL)
            return TT_READ; // No payload? Keep reading!

        do_log(LOG_DEBUG, "FOUND %d headers and payload after %ld bytes",
                cprot->headers_used, payload - cprot->buffer);

        if (cprot->headers == NULL)
        {
            int n = 0;
            cprot->headers = malloc(
                    sizeof(char *) * cprot->headers_used);
            cprot->headers_length = malloc(
                    sizeof(int) * cprot->headers_used);
            process_headers_and_get_payload(cprot, &n, headers_save);
        }

        cprot->payload = payload;
    }

    pret = check_payload_length(cprot);
    if (client->type == REQUESTER_TYPE)
    {
        int ret = -1;
        if (pret)
        {
            return TT_READ;
        }

        if (check_authorization(client))
        {
            return -1;
        }

        // we remove this header and have already changed
        // protocol from most recent to HTTP/1.0
        find_and_remove_header(cprot,
                header_connection,
                header_connection_len);

        ret = dispatch_new_connection(rena, client, cprot);
        if (ret == -3)
        {
            return -1;
        }

        return TT_READ;
    } else { // type == VICTIM_TYPE
        if (cprot->payload != NULL)
        {
            client_position_t peer_raw;
            client_position_t *peer = &peer_raw;
            int pfd = -1;

            find_and_remove_header(cprot,
                                   header_content_length,
                                   header_content_length_len);

            clients_get_peer(client, &peer_raw);
            if (peer_raw.info) pfd = clients_get_fd(peer);
            if (pfd < 0 || server_update_notify(rena, pfd, 1, 0) < 0)
            {
                do_log(LOG_DEBUG, "update notify fd [%d] failed!", pfd);
                return -1;
            }
        }

        return TT_READ;
    }

    return -1;
}
