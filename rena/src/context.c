#include "logger.h"
#include "database.h"

#include <stdlib.h>

void context_everything_allowed(struct database_object *d)
{
    context_destroy(d);
}

static int context_not_allowed(struct database_object *d, char input)
{
    (void) d;
    (void) input;
    return 0;
}

void context_nothing_allowed(struct database_object *d)
{
    context_t *c = malloc(sizeof(context_t));
    c->parser_fnc = context_not_allowed;
    database_instance_set_context(d, c);
}

void context_destroy(struct database_object *d)
{
    context_t *c = database_instance_get_context(d);
    if (c == NULL)
        return ;
    database_instance_set_context(d, NULL);
    free(c);
}
