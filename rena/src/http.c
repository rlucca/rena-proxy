
#include "global.h"
#include "http.h"
#include "database.h"
#include "task_manager.h"
#include "server.h"

#include <stdlib.h>
#include <string.h>

struct http {
    struct database_object *lookup_tree;
    int wants_write;
    int headers_used;
    const char **headers;
    int *headers_length;
    const char *payload;

    size_t buffer_used;
    char buffer[MAX_STR]; // at_least MAX_STR!! CAUTION: not finished in zero!
};

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
    ret->wants_write = type; // victim should do write first!
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

static void force_onto_buffer(client_position_t *c, const char *o, int olen)
{
    struct http *h = (struct http *) clients_get_protocol(c);
    size_t new_size = olen + h->buffer_used;
    if (new_size > MAX_STR)
    {
        size_t new_size_data = new_size - MAX_STR + sizeof(struct http);
        struct http *hl = realloc(h, new_size_data);
        if (hl != NULL)
        {
            do_log(LOG_DEBUG, "realloc done!");
            clients_set_protocol(c, hl);
            h = hl;
            if (h->headers!=NULL)
            {
                do_log(LOG_WARNING, "Client headers cleaned by realloc");
                free(h->headers);
                free(h->headers_length);
                h->headers = NULL;
                h->headers = NULL;
            }
        } else {
            do_log(LOG_ERROR, "oh nooo! realloc failed!");
            abort();
        }
    }

    memcpy(h->buffer + h->buffer_used, o, olen);
    h->buffer_used += olen;
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

    if (cfd != fd)
    {
        do_log(LOG_ERROR, "invalid client fd %d against %d (%s)",
               fd, cfd, cip);
        return -1;
    }

    if (cprot == NULL)
    {
        int is_victim = (client->type == VICTIM_TYPE);
        cprot = http_create(rena, is_victim);
        clients_set_protocol(client, cprot);
        if (cprot->wants_write)
        {
            return TT_WRITE;
        }
    }

    ret = server_read_client(cfd, cssl, buffer, &buffer_sz);
    if (ret < 0) return -1; // error?
    if (ret > 0) return ret; // ssl annoying?

    for (int i=0; i<buffer_sz; i++)
    {
        const char *transformed = NULL;
        int transformed_size = 0;
        di_output_e di = database_instance_lookup(
                                cprot->lookup_tree, buffer[i],
                                &transformed, &transformed_size);
        if (di == DBI_FEED_ME)
            continue;

        force_onto_buffer(client, transformed, transformed_size);
    }

    return 0;
}

int http_push(struct rena *rena, client_position_t *client, int fd)
{
    return -1;
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

static int check_payload_length(struct http *http)
{
    return -1;
}

static int check_authorization(client_position_t *client)
{
    return 0;
}

static int dispatch_new_connection(client_position_t *client)
{
    return -1;
}

int http_evaluate(client_position_t *client)
{
    struct http *cprot = (struct http *) clients_get_protocol(client);

    if (client->type == REQUESTER_TYPE)
    {
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

        if (check_payload_length(cprot))
        {
            return TT_READ;
        }

        if (check_authorization(client))
        {
            return -1;
        }

        if (!dispatch_new_connection(client))
        {
            cprot->wants_write = 1;
        }

        return TT_READ;
    } else { // type == VICTIM_TYPE
    }

    return -1;
}
