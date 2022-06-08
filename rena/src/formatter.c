
#include "global.h"
#include "clients.h"
#include "formatter.h"

#include <stdlib.h>

struct chain_formatter;

enum {
    DETECTED_LITERAL = 0,
    DETECTED_EXPRESSION_AND_MODIFIER,
    DETECTED_MODIFIER_ONLY,
    EXPRESSION_START,
    STATE_INVALID
};

typedef int (*handler_t)(struct chain_formatter *, struct formatter *,
                         struct formatter_userdata *);

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

struct modifier_format
{
    int group_id;
    unsigned char modifier;
    unsigned char has_expression; // 0 no, otherwise yes
    handler_t fnc;
};

static char modifier_start = '%';
static char expression_pair[] = "{}";
static struct modifier_format registered[] = {
        {  0, '%', 0, NULL }, // literal '%'
        {  1, 'h', 0, NULL },
        {  2, 'l', 0, NULL },
        {  3, 'u', 0, NULL },
        {  4, 't', 0, NULL },
        {  5, 'r', 0, NULL },
        {  6, 's', 0, NULL },
        {  7, 'b', 0, NULL },
        {  8, 'i', 1, NULL },
        { 99, 'd', 0, NULL }, // day
        { 99, 'm', 0, NULL }, // month
        { 99, 'M', 0, NULL }, // minute
        { 99, 'k', 0, NULL }, // hour
        { 99, 'S', 0, NULL }, // second
        { 99, 'Y', 0, NULL }, // year
        { 99, 'z', 0, NULL }, // timezone
        { 99, 'Z', 0, NULL }, // timezone
        { -1,   0, 0, NULL }  // copy literal and should be the last one!
    };


static int find_modifier(char ch)
{
    struct modifier_format *ptr = registered;
    for (int K=0; ptr->group_id >= 0; ptr++, K++)
    {
        if (ch == ptr->modifier)
            return K;
    }

    return -1;
}

static int evaluate_char(char c, int previous_state, int *ret_modifier)
{
    int ret = previous_state;
    // default modifier is that marked as -1 group, zero char, no expression
    int modifier = sizeof(registered) / sizeof(*registered) - 1;
    switch (previous_state)
    {
    case EXPRESSION_START:
        if (expression_pair[1] == c)
        {
            ret = DETECTED_MODIFIER_ONLY;
        } else {
            if (expression_pair[0] == c || modifier_start == c)
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
        if (modifier < 0)
            ret = STATE_INVALID;
        else
            ret = DETECTED_LITERAL;
        break;

    default: // START
        if (modifier_start == c)
            ret = DETECTED_EXPRESSION_AND_MODIFIER;
        break;
    }

    if (ret_modifier)
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
                        handler_t fnc)
{
    do_log(LOG_DEBUG, "TODO");
    return -1;
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
    int ret = -1;

    if (inout)
    {
        int state = DETECTED_LITERAL;
        size_t begin = 0;
        ret = 0;
        for (size_t pos = 0; !ret && pos < inout->format_len; pos++)
        {
            int index = -1;
            int new_state = evaluate_char(inout->format[pos], state, &index);
            if (new_state != STATE_INVALID)
            {
                // we only are interested on create an element of chain
                // when we go to literal state or get out of it
                if (state != new_state && (DETECTED_LITERAL == new_state
                    || DETECTED_EXPRESSION_AND_MODIFIER == new_state))
                {
                    if (pos == 0)
                        list_add_at_end(inout, begin, pos, index);
                    else
                        list_add_at_end(inout, begin, pos - 1, index);

                    begin = pos;
                }

                state = new_state;
            } else {
                ret = -1;
            }
        }

        if (state == DETECTED_LITERAL)
        {
        } else {
            ret = -1;
        }

        // TODO if (ret < 0) destroy everything!
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
        handler_t fnc = registered[chain->registered_index].fnc;
        if (fnc)
            return fnc(chain, inout, userdata);
    }

    return -1;
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

int formatter_destroy_handler(struct formatter **inout)
{
    if (inout)
    {
        if (list_foreach(*inout, NULL, foreach_destroy))
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
    if (inout && list_foreach(*inout, userdata, foreach_evaluate))
    {
        do_log(LOG_ERROR, "it was not possible to evaluate all chain");
        return -1;
    }

    return 0;
}
