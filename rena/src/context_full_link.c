#include "logger.h"
#include "context_full_link.h"
#include "database.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    context_t c;
    int stage;
} full_link_t;

#define STATE_UNKNOWN   0 // initialization
#define STATE_BAR       1 // found /
#define STATE_ALLOWED   2 // allowed to process
#define STATE_NONE      3 // not allowed to process
#define STATE_BAR_S     4 // found \\ previously an / go back to BAR or NONE
#define STATE_ALLOWED_S 5 // found \\ inside allowed zone go back to ALLOWED
#define STATE_NONE_S    6 // found \\ inside none zone go back to NONE or BAR

static int evaluate(char input, int *actual_stage)
{
    const char delim[] = " \t\v\n\r\f";
    const char dsafe[] = "tvnrfTVNRF";
    const char *found = NULL;
    int res = 0;
    switch (*actual_stage)
    {
        case STATE_ALLOWED:
            res = 1; // allowed to process!
            if (input == '\\')
            {
                *actual_stage = STATE_ALLOWED_S;
            } else {
                found = strchr(delim, input);
                if (found && *found != '\0')
                    *actual_stage = STATE_NONE;
            }
            break;

        case STATE_BAR:
            if (input == '/')
                *actual_stage = STATE_ALLOWED;
            else if (input == '\\')
                *actual_stage = STATE_BAR_S;
            else
                *actual_stage = STATE_NONE;
            break;

        case STATE_BAR_S:
            if (input == '/')
                *actual_stage = STATE_ALLOWED;
            else
                *actual_stage = STATE_NONE;
            break;

        case STATE_ALLOWED_S:
            res = 1; // allowed to process!
            found = strchr(dsafe, input);
            if (found && *found != '\0')
                *actual_stage = STATE_NONE;
            else
                *actual_stage = STATE_ALLOWED;
            break;

        case STATE_NONE_S:
            if (input == '/')
                *actual_stage = STATE_BAR;
            else
                *actual_stage = STATE_NONE;
            break;

        case STATE_UNKNOWN: // UNKNOWN or NONE
            *actual_stage = STATE_NONE;
            //break; /* fall down */

        default: // NONE
            if (input == '/')
                *actual_stage = STATE_BAR;
            else if (input == '\\')
                *actual_stage = STATE_NONE_S;
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
