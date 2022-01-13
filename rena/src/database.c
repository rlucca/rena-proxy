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
