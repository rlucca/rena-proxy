
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
    const char **headers;
    int *headers_length;

    size_t buffer_used;
    char buffer[MAX_STR]; // at_least MAX_STR!! CAUTION: not finished in zero!
};

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

int http_evaluate(client_position_t *client)
{
    return -1;
}
