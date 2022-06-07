
#include "global.h"
#include "clients.h"
#include "formatter.h"

typedef int (*handler_t)(char *, size_t *,
                         struct formatter *, struct formatter_userdata);

struct formatter
{
    const char *config;
    size_t position_start;
    size_t position_end;
    handler_t fnc;

    struct formatter *next;
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
        {  0, '%', 0, NULL }, // copy literal...
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
        { -1,   0, 0, NULL }
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

static int is_a_valid_input(const char *s, size_t s_len)
{
    enum {
        STATE_START = 0,
        DETECTED_EXPRESSION_AND_MODIFIER,
        DETECTED_MODIFIER_ONLY,
        EXPRESSION_START,
    };
    int ret = 0; // success
    int state = STATE_START;
    for (size_t m = 0; !ret && m < s_len; m++)
    {
        char c = s[m];

        switch (state)
        {
        case EXPRESSION_START:
            if (expression_pair[1] == c)
            {
                state = DETECTED_MODIFIER_ONLY;
            } else {
                if (expression_pair[0] == c || modifier_start == c)
                    ret = -1;
            }
            break;
        case DETECTED_EXPRESSION_AND_MODIFIER:
            if (expression_pair[0] == c)
            {
                state = EXPRESSION_START;
                continue;
            }

            /* falldown here to test modifier! */
        case DETECTED_MODIFIER_ONLY:
            if (find_modifier(c) < 0)
                ret = -1;
            else
                state = STATE_START;
            break;

        default: // START or INVALID?
            if (modifier_start == c)
                state = DETECTED_EXPRESSION_AND_MODIFIER;
            break;
        }
    }

    if (state != STATE_START)
        ret = -1;

    return ret;
}

int formatter_create_handler(struct formatter **inout,
                             const char *format, size_t format_len)
{
    //inout can be nulled to means that it will only performs validation
    if (!format || !*format || format_len == 0)
    {
        do_log(LOG_DEBUG, "invalid arguments: (%p, %p, %lu)",
               inout, format, format_len);
        return -1;
    }

    if (is_a_valid_input(format, format_len))
    {
        do_log(LOG_DEBUG, "problem parsing log format: %.*s",
               (int) format_len, format);
        return -2;
    }

    return 0;
}
