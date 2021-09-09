#define _GNU_SOURCE
#include <string.h>

#include "globals.h"
#include "http.h"

typedef enum {
    HTML_INITIALIZATION = 0,
    HTML_RECEIVED_HOST = 1,
    HTML_BODY = 4,
} html_state_e;

typedef struct {
    char *string;
    size_t reserve;
    size_t size;
} str_t;
#define RESERVE_OFFSET 1024

typedef struct {
    char host[MAX_STR];
    html_state_e stage;

    // CAUTION: not zero ending!
    str_t output;
    str_t holding;

    size_t r_lines;
    size_t n_lines;
    str_t *lines;

    void *db_lookup;

    size_t lastCharAt;
    size_t breaks;
    size_t interesting;
} http_module_t;

static char *
re_alloc(char *to_change, size_t n_elems)
{
    char *old = to_change;
    char *aux = realloc(to_change, n_elems);
    if (aux == NULL) // maybe the old support us
        return old;
    return aux;
}

static void str_resize(str_t *s, size_t mod)
{
    if ((mod + s->size) < s->reserve)
        return ;

    s->reserve += RESERVE_OFFSET;
    s->string = re_alloc(s->string, sizeof(char) * s->reserve);
}

static void str_append_char(str_t *s, char val)
{
    if (s == NULL) return ;

    str_resize(s, 1);

    s->string[s->size++] = val;
}

static void str_append_str(str_t *s, const char *val, size_t val_sz)
{
    if (s == NULL) return ;

    str_resize(s, val_sz);

    for (size_t u = 0; u < val_sz; u++)
    {
        s->string[s->size++] = val[u];
    }
}

static void add_line(http_module_t *h)
{
    const size_t offset = 4;

    if (h->lines && h->lines[h->n_lines - 1].size == 0)
    {
        return ;
    }

    h->n_lines++;
    if (h->n_lines >= h->r_lines)
    {
        h->r_lines += offset;
        h->lines = (str_t *) re_alloc((char *) h->lines,
                                      sizeof(h->lines[0]) * h->r_lines);
        memset(&h->lines[h->n_lines - 1],
               0,
               sizeof(h->lines[0]) * (h->r_lines - h->n_lines));
    }
}

void http_payload_init(fa_request_t *req)
{
    http_module_t *h = calloc(1, sizeof(http_module_t));
    h->lastCharAt = SIZE_MAX - 5;
    h->db_lookup = database_transform_handler(req->side);
    h->interesting = 1;
    add_line(h);
    req->html_state_data = h;
}

static void str_shift(str_t *ptr, int consume, char slow_mode)
{
    size_t old = ptr->size;
    if (consume > 0)
    {
        if (slow_mode)
        {
            for (int u = 0, U = ptr->size - consume, m = consume;
                    u < U; u++, m++)
            {
                ptr->string[u] = ptr->string[m];
            }
            DEBUG("SLOW MODE Done: %lu consumed %d",
                    ptr->size, consume);
        }

        ptr->size -= consume;
    }

    DEBUG("previous size %lu new size %lu consumed %d",
          old, ptr->size, consume);
}

static void accept_output(fa_request_t *req, http_module_t *h,
                          size_t length)
{
    size_t remain = fa_request_remain_capacity(req);
    if (h->output.size < remain) remain = h->output.size;
    if (remain > length) remain = length;

    int accepts = 0; // -1 pode ser setado pela funcao

    if (remain > 0)
        accepts = fa_request_payload_put(req,
                                         h->output.string,
                                         remain);

    str_shift(&h->output, accepts, remain != (size_t) accepts);
    RESTART_EV_IO(&req->write_watcher);
}

void http_payload_fill_output(fa_request_t *req,
                              size_t max_to_fill)
{
    accept_output(req, req->html_state_data, max_to_fill);
}

static size_t check_content_type(http_module_t *h)
{
    static const char ct_initials[] = "tTjJxXaA";
    static const char *content_type_list[] = {
        "text/", "javascript", "xml",
        "application/octet-stream",
        NULL
    };
    static const char content_type[] = "Content-Type";
    static const int content_type_len = 12;
    const char *value = h->lines[h->n_lines - 1].string;
    size_t value_sz = h->lines[h->n_lines - 1].size;

    if (value == NULL || value_sz < content_type_len
        || strncasecmp(value,
                       content_type, content_type_len) != 0)
    {
        // nao eh o content_type, entao nao mexe no valor
        DEBUG("RET KEEP TRANSFORM [%lu]", h->interesting);
        return h->interesting;
    }

    value = value + 12;
    while (strchr(": \t", *value) != NULL)
        value += 1;

    if (strchr(ct_initials, *value) != NULL)
    {
        for (int I=0; content_type_list[I] != NULL; I++)
        {
            if (strcasestr(value, content_type_list[I]) != NULL)
            {
                DEBUG("RET TRANSFORM [%s]", value);
                return 1;
            }
        }
    }

    DEBUG("RET DO NOT TRANSFORM [%s]", value);

    return 0;
}

int http_payload_loader(fa_request_t *req,
                        const char *buffer, size_t buffer_sz)
{
    const char seps[] = "\r\n";
    http_module_t *h = NULL;
    char *expanded = NULL; // do not free it!!!
    size_t consumed = 0; // or consumed_to_transform
    int ret = -1;

    if (req == NULL || buffer == NULL)
    {
        fprintf(stderr, "invalid parms\n");
        return -1;
    }

    h = req->html_state_data;

    for (size_t u = 0; u < buffer_sz; u++)
    {
        if (h->stage < HTML_BODY)
        {
            if (strchr(seps, buffer[u]) != NULL)
            {
                if (h->lastCharAt + 1 != u)
                {
                    h->interesting = check_content_type(h);
                    add_line(h);

                    h->lastCharAt = u;
                    h->breaks++;

                    if (h->breaks > 1)
                    {
                        h->stage = HTML_BODY;
                    }
                }
            } else {
                str_append_char(&h->lines[h->n_lines - 1], buffer[u]);
                h->breaks = 0;
            }
        }

        if (h->n_lines > 1) {
            int res;
            if (h->interesting == 0)
            {
                res = NOT_HOLD;
            } else {
                res = database_transform_lookup(
                                        &h->db_lookup,
                                        buffer[u],
                                        0, &expanded, &consumed);
            }

            if (res == NOT_HOLD)
            {
                str_resize(&h->output, h->holding.size + 1);

                if (h->holding.size > 0)
                {
                    str_append_str(&h->output, h->holding.string,
                                   h->holding.size);
                    h->holding.size = 0;
                }

                str_append_char(&h->output, buffer[u]);
                ret = 0;
            } else if (res == FEED_ME_AND_HOLD)
            {
                str_append_char(&h->holding, buffer[u]);
                if (ret < 0) ret = 1;
            } else if (res == TRANSFORM_OK)
            {
                size_t len = strlen(expanded);

                str_resize(&h->output, h->holding.size + len);

                if (h->holding.size > 0)
                {
                    h->holding.size -= consumed - 1;
                    str_append_str(&h->output, h->holding.string,
                                   h->holding.size);
                    h->holding.size = 0;
                }

                if (len > 0)
                {
                    str_append_str(&h->output, expanded, len);
                }

                ret = 0;
            }
        } else {
            str_append_char(&h->output, buffer[u]);
        }
    }

#if 0
    DEBUG("--- SIDE [%d] F OUTPUT |%.*s| %lu",
          req->side, (int) h->output.size, h->output.string,
          h->output.size);

    DEBUG("--- SIDE [%d] F HOLDIG |%.*s| %lu",
          req->side, (int) h->holding.size, h->holding.string,
          h->holding.size);

    for (size_t I=0; I<h->n_lines; I++)
    {
        DEBUG("--- SIDE [%d] F HEADER %lu |%.*s| %lu",
                req->side, I, (int) h->lines[I].size, h->lines[I].string,
                h->lines[I].size);
    }
#endif
    if (req->related_fd >= 0)
    {
		fa_request_t *related = requests[req->related_fd];
        DEBUG("Enfileirado pedido de escrita para o canal %d vindo do %d: %p",
              req->related_fd, req->fd, related);

        accept_output(related, h, SIZE_MAX - 1);
        return ret;
    }

    char *hst = strstr(h->output.string, "Host: ");
    char *brkr = strchr(hst, '\r');
    char *brkn = strchr(hst, '\n');
    char *brk = (brkr < brkn) ? brkr : brkn;

    // copy Host's value to internal structure...
    snprintf(h->host, sizeof(h->host), "%.*s",
             (int) (brk - hst - 6), hst + 6);

    int comm_port = (req->ctx == NULL) ? HTTP_PORT : HTTPS_PORT;
    req->related_fd = connect_to_server(h->host, comm_port);

    if (req->related_fd < 0)
    {
        h->host[0] = '\0';
        DEBUG("Derrubando cliente por nao conseguir conectar...");
        //fa_request_delete(req); // delete is on service_thread
        ret = -1;
    } else {
		char is_https = (req->ctx != NULL) ? 's' : '\0';
		fa_request_t *related = accept_action_internal(is_https, req->related_fd, 1);
		if (related == NULL)
		{
            DEBUG("Derrubando cliente por nao conseguir fechar handshake...");
            //fa_request_delete(req); // delete is on service_thread
            fa_request_close_socket(req->related_fd);
            req->related_fd = -1;
            ret = -1;
		} else {
            accept_output(related, h, SIZE_MAX - 1);
			related->related_fd = req->fd;
		}
    }

    return ret;
}

fa_request_t *http_payload_holding_loader(fa_request_t *req)
{
    http_module_t *h = NULL;

    if (req == NULL)
    {
        fprintf(stderr, "invalid parms\n");
        return NULL;
    }

    h = req->html_state_data;

    fa_request_t *related = requests[req->related_fd];
    if (related == NULL)
    {
        fprintf(stderr, "related nao conseguiu ser recuperado\n");
        return NULL;
    }

    size_t remain = fa_request_remain_capacity(related);
    if (h->holding.size < remain) remain = h->holding.size;

    int accepts = 0;

    if (remain > 0)
        accepts = fa_request_payload_put(related,
                                         h->holding.string,
                                         remain);
    str_shift(&h->holding, accepts, remain != (size_t) accepts);
    RESTART_EV_IO(&related->write_watcher);
    return (h->holding.size == 0) ? NULL : related;
}

int http_payload_get_header(fa_request_t *req, char *name,
                            char *value, size_t value_sz)
{
    http_module_t *h = (http_module_t *) req->html_state_data;

    for (size_t u = 1; u < h->n_lines; u++)
    {
        if (!strncmp(h->lines[u].string, name, strlen(name)))
        {
            int ret = snprintf(value, value_sz, "%.*s",
                        (int) h->lines[0].size, h->lines[0].string);

            return (ret > 0) ? 1 : 0;
        }
    }

    return 0;
}

int http_payload_split_first_line(fa_request_t *req,
                                  char *method, char *path,
                                  char *protocol)
{
    http_module_t *h = (http_module_t *) req->html_state_data;
    int ret = sscanf(h->lines[0].string,
                     " %s %s %s",
                     method, path, protocol);

    return (ret > 0) ? 1 : 0;
}

int http_payload_get_first_line(fa_request_t *req,
                                char *value, size_t value_sz)
{
    http_module_t *h = (http_module_t *) req->html_state_data;

    int ret = snprintf(value, value_sz, "%.*s",
                       (int) h->lines[0].size, h->lines[0].string);

    return (ret > 0) ? 1 : 0;
}

int http_payload_release(fa_request_t *req)
{
    http_module_t *h = req->html_state_data;
    database_transform_cleanup(&h->db_lookup);
    free(h->output.string);
    free(h->holding.string);
    // LATER lembrar que antes de acabar com o holding/output, tem que pegar o holding
    // que ainda fica pendente se nao pode faltar o ultimo bloco da pagina!
    // TODO first line e headers/values
    // se for so ter um retorno entao virar pra void!
    return 0;
}
