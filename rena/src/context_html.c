#include "logger.h"
#include "context_html.h"
#include "database.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    context_t c;
    int stage;
    int ignore;
} html_t;

#define STATE_UNKNOWN              0 // initialization
#define STATE_OUTSIDE_TAG          1
#define STATE_INSIDE_TAG           2
#define STATE_INSIDE_TAG_0         3 // first <
#define STATE_INSIDE_TAG_1         4 // found <!
#define STATE_INSIDE_TAG_2         5 // found <!-
#define STATE_INSIDE_COMM_0        6
#define STATE_INSIDE_COMM_1        7 // found -
#define STATE_INSIDE_COMM_2        8 // found --+>
#define STATE_INSIDE_ATTRIB        9 // waiting for any " \t\v\n\r\f>"
#define STATE_INSIDE_ATTRIB_IGN   10 // found a protective char
#define STATE_INSIDE_ATTRIB_0     11 // waiting for any " \t\v\n\r\f>"
#define STATE_INSIDE_ATTRIB_S     12 // waiting for '
#define STATE_INSIDE_ATTRIB_S_IGN 13 // found a protective char
#define STATE_INSIDE_ATTRIB_D     14 // waiting for "
#define STATE_INSIDE_ATTRIB_D_IGN 15 // found a protective char

static int inside_tag(char input, int *actual_stage, int *ignore_flag)
{
    int res = (*ignore_flag) ? 0 : 1;

    if (input == '>')
    {
        *actual_stage = STATE_OUTSIDE_TAG;
    }
    else if (input == '=')
    {
        *actual_stage = STATE_INSIDE_ATTRIB_0;
    }
    else if (*actual_stage == STATE_INSIDE_TAG_0)
    {
        if (input == '!')
            *actual_stage = STATE_INSIDE_TAG_1;
        else
            *actual_stage = STATE_INSIDE_TAG;
    } else if (*actual_stage == STATE_INSIDE_TAG_1
                || *actual_stage == STATE_INSIDE_TAG_2)
    {
        if (input == '-')
            *actual_stage += 1; // INSIDE_TAG_2 and INSIDE_COMM_0
        else
            *actual_stage = STATE_INSIDE_TAG;
    }

    return res;
}

static int inside_attribute(char input, int *actual_stage, int *ignore_flag)
{
    // valid examples
    //      <tag number=4 />
    //      <tag number="4" />
    //      <tag number='4' />
    //      <tag phrase='4 a \'4" ' />
    //      <tag phrase="4 a '4\" " />
    const char common[] = " \t\v\n\r\f>";
    const char simple_q[] = "'";
    const char double_q[] = "\"";
    const char *delim = common;
    const char *found = NULL;
    int res = 1;
    *ignore_flag = 0;

    if (*actual_stage == STATE_INSIDE_ATTRIB_S)
        delim = simple_q;
    else if (*actual_stage == STATE_INSIDE_ATTRIB_D)
        delim = double_q;

    if (*actual_stage == STATE_INSIDE_ATTRIB_0)
    {
        if (input == '\'')
            *actual_stage = STATE_INSIDE_ATTRIB_S;
        else if (input == '"')
            *actual_stage = STATE_INSIDE_ATTRIB_D;
        else
            *actual_stage = STATE_INSIDE_ATTRIB;
    }

    if (input == '\\')
    {
        *actual_stage += 1; // GO TO *_IGN
    } else {
        // protective char is handled on evaluate...
        found = strchr(delim, input);

        if (found != NULL && *found != '\0')
        {
            *actual_stage = STATE_INSIDE_TAG;
        }
    }

    return res;
}

static int inside_comment(char input, int *actual_stage, int *ignore_flag)
{
    int res = (*ignore_flag) ? 0 : 1;

    switch (*actual_stage)
    {
        case STATE_INSIDE_COMM_2:
            if (input == '>')
                *actual_stage = STATE_OUTSIDE_TAG;
            else if (input != '-')
                *actual_stage = STATE_INSIDE_COMM_0;
            break;
        case STATE_INSIDE_COMM_1:
            if (input == '-')
                *actual_stage = STATE_INSIDE_COMM_2;
            else
                *actual_stage = STATE_INSIDE_COMM_0;
            break;
        default: // STATE_INSIDE_COMM_0
            if (input == '-')
                *actual_stage = STATE_INSIDE_COMM_1;
            break;
    }

    return res;
}

static int outside_tag(char input, int *actual_stage, int *ignore_flag)
{
    const char *found = strchr(" \t\v\n\r\f<", input);
    int res = (*ignore_flag) ? 0 : 1;
    if (found && *found != '\0')
    {
        if (input == '<')
        {
            *ignore_flag = 1;
            *actual_stage = STATE_INSIDE_TAG_0;
        }
    }

    return res;
}

static int evaluate(char input, int *actual_stage, int *ignore)
{
    int res = (*ignore) ? 0 : 1;

    switch (*actual_stage)
    {
        case STATE_INSIDE_ATTRIB:
        case STATE_INSIDE_ATTRIB_0:
        case STATE_INSIDE_ATTRIB_S:
        case STATE_INSIDE_ATTRIB_D:
            res = inside_attribute(input, actual_stage, ignore);
            break;
        case STATE_INSIDE_ATTRIB_IGN:
            *actual_stage = STATE_INSIDE_ATTRIB;
            break;
        case STATE_INSIDE_ATTRIB_S_IGN:
            *actual_stage = STATE_INSIDE_ATTRIB_S;
            break;
        case STATE_INSIDE_ATTRIB_D_IGN:
            *actual_stage = STATE_INSIDE_ATTRIB_D;
            break;
        case STATE_INSIDE_TAG:
        case STATE_INSIDE_TAG_0:
        case STATE_INSIDE_TAG_1:
        case STATE_INSIDE_TAG_2:
            res = inside_tag(input, actual_stage, ignore);
            break;
        case STATE_INSIDE_COMM_0:
        case STATE_INSIDE_COMM_1:
        case STATE_INSIDE_COMM_2:
            res = inside_comment(input, actual_stage, ignore);
            break;
        case STATE_UNKNOWN:
        case STATE_OUTSIDE_TAG:
            res = outside_tag(input, actual_stage, ignore);
            break;
        default:
            break;
    };

    return res;
}

static int public_entry(struct database_object *d, char input)
{
    html_t *c = (html_t *) database_instance_get_context(d);
    return evaluate(input, &c->stage, &c->ignore);
}

void context_set_html_parser(struct database_object *d)
{
    html_t *h = (html_t *) calloc(1, sizeof(html_t));
    h->ignore = 1;
    context_t *c = (context_t *) h;
    c->parser_fnc = public_entry;
    database_instance_set_context(d, c);
}
