#include "logger.h"
#include "context_full_link.h"
#include "database.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    context_t c;
    int stage;
} full_link_t;

#define STATE_UNKNOWN 0 // initialization
#define STATE_BAR     1 // found /
#define STATE_2BAR    2 // found //
#define STATE_ALLOWED 3 // allowed to process
#define STATE_NONE    4 // not allowed to process

static int evaluate(char input, int *actual_stage)
{
    const char delim[] = " \t\v\n\r\f";
    const char *found = NULL;
    int res = 0;
    switch (*actual_stage)
    {
        case STATE_ALLOWED:
            res = 1; // allowed to process!
            found = strchr(delim, input);
            if (found && *found != '\0')
                *actual_stage = STATE_NONE;
            break;

        case STATE_BAR:
        case STATE_2BAR:
            if (input == '/')
                *actual_stage += 1; // go to 2BAR or ALLOWED
            else
                *actual_stage = STATE_NONE;
            break;

        case STATE_UNKNOWN:
            *actual_stage = STATE_NONE;
            //break; /* fall down */

        default: // UNKNOWN or NONE
            if (input == '/')
                *actual_stage = STATE_BAR;
            break;
    };

    return res;
}

static int public_entry(struct database_object *d, char input)
{
    full_link_t *c = (full_link_t *) database_instance_get_context(d);
    return evaluate(input, &c->stage);
}

// depends from \/\/ until a space is found
void context_set_full_link_parser(struct database_object *d)
{
    context_t *c = (context_t *) calloc(1, sizeof(full_link_t));
    c->parser_fnc = public_entry;
    database_instance_set_context(d, c);
}
