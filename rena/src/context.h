#ifndef CONTEXT_H_
#define CONTEXT_H_

struct database_object; // forward

typedef struct {
    // (0) not allowed to process; (1) allowed to process;
    int (*parser_fnc)(struct database_object *d, char input);
} context_t;

void context_everything_allowed(struct database_object *d);
void context_nothing_allowed(struct database_object *d);
void context_destroy(struct database_object *d);

#endif
