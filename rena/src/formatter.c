
#include "global.h"
#include "clients.h"
#include "formatter.h"

#include <stdlib.h>
#include <string.h>

enum {
    DETECTED_LITERAL = 0,
    DETECTED_EXPRESSION_AND_MODIFIER,
    DETECTED_MODIFIER_ONLY,
    EXPRESSION_START,
    STATE_INVALID
};

struct chain_formatter
{
    size_t position_start;
    size_t position_end;
    int registered_index;

    struct chain_formatter *next;
};

struct formatter
{
    const char *format;
    size_t format_len;

    struct chain_formatter *first;
    struct chain_formatter *last;
};


// handler of modifiers {
typedef int (*format_callback_t)(struct chain_formatter *,
                                 struct formatter *,
                                 struct formatter_userdata *);

#define DECLARE(xname) static int xname(struct chain_formatter *, \
          struct formatter *, struct formatter_userdata *)

DECLARE(modifier_copy_literal);
DECLARE(modifier_percentage);
DECLARE(modifier_none);
DECLARE(modifier_get_header);
DECLARE(modifier_authenticated_user);
DECLARE(modifier_requester_ip);
DECLARE(modifier_requester_requisition);
DECLARE(modifier_requester_status_code);
DECLARE(modifier_requester_formatted_date);
DECLARE(modifier_sum_of_bytes_transferred);

#undef DECLARE
// } handler of modifiers

struct modifier_format
{
    unsigned char modifier;
    unsigned char has_expression; // 0 no, otherwise yes
    format_callback_t fnc;
};

static char modifier_start = '%';
static char expression_pair[] = "{}";
static struct modifier_format registered[] = {
        {  '%', 0, modifier_percentage },
        {  'h', 0, modifier_requester_ip },
        {  'l', 0, modifier_none },
        {  'u', 0, modifier_authenticated_user },
        {  't', 1, modifier_requester_formatted_date },
        {  'r', 0, modifier_requester_requisition },
        {  's', 0, modifier_requester_status_code },
        {  'b', 0, modifier_sum_of_bytes_transferred },
        {  'i', 1, modifier_get_header },
        {    0, 0, modifier_copy_literal } // must be the last!
    };


static int find_modifier(char ch)
{
    struct modifier_format *ptr = registered;
    for (int K=0; ptr->modifier != 0; ptr++, K++)
    {
        if (ch == ptr->modifier)
            return K;
    }

    return -1;
}

static int evaluate_char(char c, int previous_state, int *ret_modifier)
{
    int ret = previous_state;
    int modifier = -1;
    switch (previous_state)
    {
    case EXPRESSION_START:
        if (expression_pair[1] == c)
        {
            ret = DETECTED_MODIFIER_ONLY;
        } else {
            if (expression_pair[0] == c)
                ret = STATE_INVALID;
        }
        break;
    case DETECTED_EXPRESSION_AND_MODIFIER:
        if (expression_pair[0] == c)
        {
            ret = EXPRESSION_START;
            break;
        }

        /* falldown here to test modifier! */
    case DETECTED_MODIFIER_ONLY:
        modifier = find_modifier(c);
#define CHECK_INVALID_EXPRESSION(x) (previous_state == DETECTED_MODIFIER_ONLY \
                                    && registered[x].has_expression == 0)
        if (modifier < 0 || CHECK_INVALID_EXPRESSION(modifier))
            ret = STATE_INVALID;
        else
            ret = DETECTED_LITERAL;
#undef CHECK_INVALID_EXPRESSION
        break;

    default: // START or INVALID
        if (modifier_start == c)
            ret = DETECTED_EXPRESSION_AND_MODIFIER;
        break;
    }

    if (ret_modifier && modifier >= 0)
        *ret_modifier = modifier;

    return ret;
}

static int is_a_valid_input(const char *s, size_t s_len)
{
    int ret = 0; // success
    int state = DETECTED_LITERAL;
    for (size_t m = 0; !ret && m < s_len; m++)
    {
        char c = s[m];
        int new_state = evaluate_char(c, state, NULL);
        if (new_state != STATE_INVALID)
            state = new_state;
        else
            ret = -1;
    }

    if (state != DETECTED_LITERAL)
        ret = -1;

    return ret;
}

static void list_add_at_end(struct formatter *inout,
                            size_t begin, size_t end, int index)
{
    struct chain_formatter *a;
    a = malloc(sizeof(*a));
    a->position_start = begin;
    a->position_end = end;
    a->registered_index = index;
    a->next = NULL;

    if (inout->first == NULL)
    {
        inout->first = inout->last = a;
        return ;
    }

    inout->last->next = a;
    inout->last = a;
}

static int list_foreach(struct formatter *inout,
                        struct formatter_userdata *userdata,
                        format_callback_t fnc)
{
    struct chain_formatter *chain = NULL;
    int ret = 0;

    if (inout == NULL)
    {
        do_log(LOG_ERROR, "call create handler before!");
        return -1;
    }

    if (fnc == NULL)
    {
        do_log(LOG_ERROR, "no handler to be used informed!");
        return -2;
    }

    chain = inout->first;
    while (!ret && chain)
    {
        struct chain_formatter *temp = chain;
        chain = chain->next;
        if (fnc(temp, inout, userdata))
            ret = -1;
    }

    return ret;
}

static int foreach_destroy(struct chain_formatter *chain,
                           struct formatter *inout,
                           struct formatter_userdata *userdata)
{
    (void) inout;
    (void) userdata;
    free(chain);
    return 0;
}

static int list_destroy(struct formatter *inout)
{
    int ret = list_foreach(inout, NULL, foreach_destroy);
    if (!ret)
    {
        inout->first = NULL;
        inout->last = NULL;
    }
    return ret;
}

static struct formatter *create_list_head(const char *format,
                                          size_t format_len)
{
    struct formatter *ret = NULL;
    ret = malloc(sizeof(*ret));
    ret->format = format;
    ret->format_len = format_len;
    ret->first = ret->last = NULL;
    return ret;
}

static int create_chain_from_format(struct formatter *inout)
{
    int default_modifier = sizeof(registered) / sizeof(*registered) - 1;
    int ret = -1;

    if (inout)
    {
        int state = DETECTED_LITERAL;
        size_t begin = 0;
        ret = 0;
        for (size_t pos = 0; !ret && pos < inout->format_len; pos++)
        {
            int index = default_modifier;
            int new_state = evaluate_char(inout->format[pos], state, &index);
            if (new_state != STATE_INVALID)
            {
                if (state != new_state && (DETECTED_LITERAL == new_state
                    || (DETECTED_EXPRESSION_AND_MODIFIER == new_state
                        && begin != pos)))
                {
                    int mod = pos;
                    if (DETECTED_EXPRESSION_AND_MODIFIER == new_state)
                        mod--;
                    list_add_at_end(inout, begin, mod, index);
                    begin = mod + 1;
                }

                state = new_state;
            } else {
                ret = -1;
            }
        }

        if (state == DETECTED_LITERAL)
        {
            if (begin < inout->format_len)
            {
                int index = default_modifier;
                int flen = inout->format_len;
                if (flen > 0) flen--;
                list_add_at_end(inout, begin, flen, index);
            }

        } else {
            ret = -1;
        }

        if (ret < 0)
        {
            (void) list_destroy(inout);
        }
    }

    // alem disso, se ouver agrupadores iguais intercalados por agrupador zero
    // eles devem ser considerados como somente um unico elemento da cadeia!
    return ret;
}

static int parse_input(struct formatter **inout,
                       const char *format, size_t format_len)
{
    if (is_a_valid_input(format, format_len))
        return -1;

    if (inout != NULL)
    {
        *inout = create_list_head(format, format_len);
        if (create_chain_from_format(*inout))
        {
            formatter_destroy_handler(inout);
            return -2;
        }
    }

    return 0;
}

int formatter_create_handler(struct formatter **inout,
                             const char *format, size_t format_len)
{
    //inout can be nulled to means that it will only performs validation
    if ((inout && *inout) || !format || !*format || format_len == 0)
    {
        do_log(LOG_DEBUG, "invalid arguments: (%p, %p, %lu)",
               inout, format, format_len);
        return -1;
    }

    if (parse_input(inout, format, format_len))
    {
        do_log(LOG_DEBUG, "problem parsing log format: %.*s",
               (int) format_len, format);
        return -2;
    }

    return 0;
}

static int foreach_evaluate(struct chain_formatter *chain,
                            struct formatter *inout,
                            struct formatter_userdata *userdata)
{
    if (chain->registered_index >= 0)
    {
        format_callback_t fnc
                = registered[chain->registered_index].fnc;
        if (fnc)
            return fnc(chain, inout, userdata);
    }

    return -1;
}

int formatter_destroy_handler(struct formatter **inout)
{
    if (inout)
    {
        if (list_destroy(*inout))
        {
            do_log(LOG_ERROR, "it was not possible to free all data");
        }

        free(*inout);
        *inout = NULL;
    }

    return 0;
}

int formatter_evaluate(struct formatter **inout,
                       struct formatter_userdata *userdata)
{
    if (!userdata || !userdata->client || !userdata->out)
    {
        do_log(LOG_ERROR,
               "it was not possible to evaluate. Missing "
               "client information or output buffer");
        return -1;
    }

    if (inout && list_foreach(*inout, userdata, foreach_evaluate))
    {
        do_log(LOG_ERROR, "it was not possible to evaluate all chain");
        return -1;
    }

    return 0;
}


// HANDLE OF MODIFIERS


static int modifier_copy_literal(struct chain_formatter *cf,
                          struct formatter *fo,
                          struct formatter_userdata *fu)
{
    int size = cf->position_end - cf->position_start + 1;

    if (fu->out_len + size > fu->out_sz
        || cf->position_start >= fo->format_len
        || cf->position_end >= fo->format_len)
    {
        return -1;
    }


    if (memcpy(fu->out + fu->out_len,
               fo->format + cf->position_start,
               size) == NULL)
    {
        return -1;
    }

    fu->out_len += size;
    return 0;
}

static int copy_literal_char(struct formatter_userdata *fu, char l)
{
    int size = 1;

    if (fu->out_len + size > fu->out_sz)
    {
        return -1;
    }

    *(fu->out + fu->out_len) = l;
    fu->out_len += size;
    return 0;
}

static int modifier_percentage(struct chain_formatter *cf,
                        struct formatter *fo,
                        struct formatter_userdata *fu)
{
    return copy_literal_char(fu, '%');
}

static int modifier_none(struct chain_formatter *cf,
                  struct formatter *fo,
                  struct formatter_userdata *fu)
{
    return copy_literal_char(fu, '-');
}

static int call_strftime(struct formatter_userdata *fu,
                         const char *format)
{
    struct tm tm;
    const time_t *t = clients_get_timestamp(fu->client);
    size_t ret = strftime(fu->out + fu->out_len,
                          fu->out_sz - fu->out_len,
                          format, localtime_r(t, &tm));
    if (ret == 0)
        return -1; // fu->out is invalid from here
    fu->out_len += ret;
    return 0;
}

static int modifier_requester_formatted_date(struct chain_formatter *cf,
                                      struct formatter *fo,
                                      struct formatter_userdata *fu)
{
    char format[MAX_STR] = "[%d/%b/%Y:%H:%M:%S %z]";

    if (fo->format[cf->position_start + 1] == '{')
    {
        int diff = cf->position_end - cf->position_start - 3;
        if (diff > sizeof(format))
            return -1;

        snprintf(format, sizeof(format),
                 "%.*s", diff,
                 fo->format + cf->position_start + 2);
    }

    return call_strftime(fu, format);
}

static int modifier_authenticated_user(struct chain_formatter *cf,
                                struct formatter *fo,
                                struct formatter_userdata *fu)
{
    return copy_literal_char(fu, '-');
}

static int call_client_log_format(client_log_format_t *clf,
                                  struct formatter_userdata *fu,
                                  char default_value)
{
    int ret = client_do_log_format(clf);
    if (ret >= 0)
    {
        fu->out_len += ret;
        return 0;
    }

    if (fu->out_len < fu->out_sz)
    {
        fu->out[fu->out_len] = default_value;
        fu->out_len++;
        return 0;
    }

    return -1;
}

static int modifier_sum_of_bytes_transferred(
                                    struct chain_formatter *cf,
                                    struct formatter *fo,
                                    struct formatter_userdata *fu)
{
    client_log_format_t format = {
            LOG_FORMAT_BYTES_RECEIVED,
            fu->client, NULL,
            fu->out + fu->out_len,
            fu->out_sz - fu->out_len,
            0
        };

    return call_client_log_format(&format, fu, '0');
}

static int modifier_requester_status_code(struct chain_formatter *cf,
                                   struct formatter *fo,
                                   struct formatter_userdata *fu)
{
    client_log_format_t format = {
            LOG_FORMAT_STATUS_CODE,
            fu->client, NULL,
            fu->out + fu->out_len,
            fu->out_sz - fu->out_len,
            0
        };

    return call_client_log_format(&format, fu, '-');
}

static int modifier_requester_requisition(
                                   struct chain_formatter *cf,
                                   struct formatter *fo,
                                   struct formatter_userdata *fu)
{
    client_log_format_t format = {
            LOG_FORMAT_REQUEST_LINE,
            fu->client, NULL,
            fu->out + fu->out_len,
            fu->out_sz - fu->out_len,
            0
        };

    return call_client_log_format(&format, fu, '-');
}

static int modifier_get_header(struct chain_formatter *cf,
                        struct formatter *fo,
                        struct formatter_userdata *fu)
{
    //35 44 => %{header}i ERASEME TODO
    int length = cf->position_end - cf->position_start - 3;
    client_log_format_t format = {
            LOG_FORMAT_HEADER_VALUE,
            fu->client,
            fo->format + cf->position_start + 2,
            fu->out + fu->out_len,
            fu->out_sz - fu->out_len,
            length
        };

    return call_client_log_format(&format, fu, '-');
}

static int modifier_requester_ip(struct chain_formatter *cf,
                          struct formatter *fo,
                          struct formatter_userdata *fu)
{
    client_log_format_t format = {
            LOG_FORMAT_REQUEST_IP_CLIENT,
            fu->client, NULL,
            fu->out + fu->out_len,
            fu->out_sz - fu->out_len,
            0
        };

    return call_client_log_format(&format, fu, '-');
}
