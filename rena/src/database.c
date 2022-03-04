#include "global.h"
#include "database.h"
#include "database_reader.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>

struct database
{
    const char *dpath;
    const char *suffix;
    tree_node_t *rules[DB_LAST];
};

struct database_object_pair
{
    tree_node_t *actual;
    struct database_object_pair *next;
    int depth;
};

struct database_object
{
    tree_node_t *never_rules;
    tree_node_t *side_rules;
    struct database_object_pair *list;
    const char *transformation;
    char *input; // not zero-ended
    int input_sz;
    int input_rs;
    int transformation_consume;
};


static int filename_valid(const char *filename)
{
    size_t len = strlen(filename) - 3;

    if (filename == NULL || *filename == '_'
        || strcmp(filename + len, "txt"))
    {
        do_log(LOG_DEBUG, "Ignoring filename: %s", filename);
        return 1;
    }

    return 0;
}

static void rename_domain(struct database *db, char *params)
{
    char victim[MAX_STR];
    char *first_space = strchr(params, ' ');
    char *second_space = NULL;
    char *second_param = NULL;

    if (first_space != NULL)
    {
        *first_space = '\0';
        second_param = first_space + 1;
        second_space = strchr(second_param, ' ');
        if (second_space != NULL)
        {
            *second_space = '\0';
        }
    }

    snprintf(victim, MAX_STR, "%s%s", second_param, db->suffix);

    do_log(LOG_DEBUG, "Adding rule to transform [%s] to [%s]",
           params, victim);

    db->rules[DB_TO_SERVER] = tree_insert(db->rules[DB_TO_SERVER],
                                          victim, strlen(victim),
                                          params);
    db->rules[DB_FROM_SERVER] = tree_insert(db->rules[DB_FROM_SERVER],
                                          params, strlen(params),
                                          victim);
}

static void no_proxy(struct database *db, char *params)
{
    char *first_space = strchr(params, ' ');

    if (first_space != NULL)
    {
        *first_space = '\0';
    }

    do_log(LOG_DEBUG, "Adding rule to NEVER PROXY [%s]", params);

    db->rules[DB_NO_PROXY] = tree_insert(db->rules[DB_NO_PROXY],
                                          params, strlen(params),
                                          NULL);
}

static void read_line(struct rena *rena, char *head, char *tail)
{
    struct {
        const char *head;
        void (*fnc)(struct database *, char *);
    } *ptr, ha[] = {
            {"proxy_domain_rename", rename_domain},
            {"ProxyHostnameEdit", rename_domain},
            {"NeverProxy", no_proxy},
            {"no_proxy", no_proxy},
            {NULL, NULL}
        };
    struct database *db = rena->db;

    for (ptr = ha; ptr->head != NULL; ptr++)
    {
        if (!strcmp(head, ptr->head))
        {
            if (ptr->fnc)
            {
                ptr->fnc(db, tail);
            }
            break;
        }
    }
}

struct database *database_init(struct rena *modules)
{
    struct handler_db_reader rd = {
        filename_valid, read_line
    };
    int ret = 0;

    if (modules->db != NULL)
    {
        return modules->db;
    }

    modules->db = calloc(1, sizeof(struct database));

    config_get_database_directory(&modules->config,
                                  &modules->db->dpath);
    config_get_database_suffix(&modules->config,
                               &modules->db->suffix);

    ret = database_reader(modules->db->dpath, &rd, modules);
    if (ret != 0)
    {
        database_free(modules);
    }

    return modules->db;
}

void database_free(struct rena *modules)
{
    struct database *db = modules->db;
    for (int i=0; i < DB_LAST; i++)
    {
        tree_destroy(&db->rules[i]);
    }
    free(db);
    modules->db = NULL;
}

static void di_pair_destroy(struct database_object_pair *dop)
{
    struct database_object_pair *aux = NULL;
    while (dop)
    {
        free(aux);
        aux = dop;
        dop = dop->next;
    }

    free(aux);
}

struct database_object *database_instance_create(struct rena *rena,
                                                 int is_victim)
{
    struct database *db = rena->db;
    struct database_object *ret = calloc(
                                    1,
                                    sizeof(struct database_object));
    int side = DB_TO_SERVER;
    if (is_victim) side = DB_FROM_SERVER;
    ret->never_rules = db->rules[DB_NO_PROXY];
    ret->side_rules = db->rules[side];
    return ret;
}

static struct database_object_pair *dbo_pair_create(tree_node_t *t)
{
    struct database_object_pair *ret = malloc(
                                        sizeof(struct database_object_pair));
    ret->next = NULL;
    ret->actual = t;
    ret->depth = 1;
    return ret;
}

static struct database_object_pair *dbo_list_remove(
        struct database_object *d,
        struct database_object_pair *prev,
        struct database_object_pair *current)
{
    struct database_object_pair *ret = current->next;
    if (prev)
        prev->next = ret;
    else
        d->list = ret;
    free(current);
    return ret;
}

static void dbo_list_prepend(struct database_object *d, tree_node_t *t)
{
    if (t == NULL || d == NULL)
        return ;
    struct database_object_pair *dbop = dbo_pair_create(t);
    dbop->next = d->list;
    d->list = dbop;
}

static void add_input(struct database_object *d, char input)
{
    if (1 + d->input_sz >= d->input_rs)
    {
        const int BLOCK = 16;
        char *move = realloc(d->input, d->input_rs + BLOCK);
        if (move != NULL)
        {
            d->input_rs += BLOCK;
            d->input = move;
        }
    }

    d->input[d->input_sz] = input;
    d->input_sz += 1;
}

static void clear_input(struct database_object *d)
{
    d->input_sz = 0;
}

static void consume_input(struct database_object *d, int x)
{
    if (x >= d->input_sz)
    {
        if (x > d->input_sz)
        {
            do_log(LOG_ERROR,
                   "Consuming more input than I have (%d > %d)",
                   x, d->input_sz);
            abort(); // during tests
        }
        d->input_sz = 0;
    } else { // x < size
        d->input_sz -= x;
        memmove(d->input,
                d->input + x,
                d->input_sz);
    }
}

static void destroy_input(struct database_object *d)
{
    free(d->input);
    d->input = NULL;
    d->input_rs = d->input_sz = 0;
}

void database_instance_destroy(struct database_object **d)
{
    if (d == NULL)
        return ;

    di_pair_destroy((*d)->list);
    (*d)->list = NULL;
    destroy_input(*d);
    // DO NOT FREE never_rules or side_rules!!!
    free(*d);
    *d = NULL;
}

static void dbo_list_foreach(struct database_object *d,
                             char input)
{
    for (struct database_object_pair *lookup=d->list, *prev=NULL;
            lookup != NULL; )
    {
        tree_node_t *aux = tree_get_child(lookup->actual, input);
        if (aux == NULL)
        {
            lookup = dbo_list_remove(d, prev, lookup);
            continue;
        }

        lookup->depth += 1;
        if (aux->adapted != NULL)
        {
            if (d->transformation_consume == 0
                    || d->transformation_consume < lookup->depth)
            {
                d->transformation_consume = lookup->depth;
                d->transformation = aux->adapted;
            }
        }
        lookup->actual = aux;

        prev = lookup;
        lookup = lookup->next;
    }
}

di_output_e database_instance_lookup(struct database_object *d,
                                     char input,
                                     const char ** const o,
                                     int * const olen)
{
    int empty = 1;
    dbo_list_foreach(d, input);

    if (d->list != NULL)
        empty = 0;

    add_input(d, input);
    dbo_list_prepend(d, tree_get_sibling(d->never_rules, input));
    dbo_list_prepend(d, tree_get_sibling(d->side_rules, input));

    if (empty && d->transformation_consume > 0)
    {
        *o = d->transformation;
        *olen = strlen(d->transformation);
        consume_input(d, d->transformation_consume);
        d->transformation_consume = 0;
        d->transformation = NULL;
        return DBI_TRANSFORMATION_FOUND;
    }

    if (d->list == NULL)
    {
        database_instance_get_holding(d, o, olen);
        return DBI_NOT_HOLD;
    }

    return DBI_FEED_ME;
}

void database_instance_dump(struct database_object *d)
{
    tree_dump(d->never_rules);
    tree_dump(d->side_rules);
}

void database_instance_get_holding(struct database_object *d,
                                   const char ** const o,
                                   int * const olen)
{
    if (d->list == NULL)
    {
        *o = d->input;
        *olen = d->input_sz;
        clear_input(d);
    }
}
