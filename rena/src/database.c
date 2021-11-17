#include "global.h"
#include "database.h"
#include "tree.h"
#include <stdlib.h>

struct database
{
    const char *dpath;
    const char *suffix;
};

struct database *database_init(struct rena *modules)
{
    if (modules->db != NULL)
    {
        return modules->db;
    }

    modules->db = calloc(1, sizeof(struct database));

    config_get_database_directory(&modules->config,
                                  &modules->db->dpath);
    config_get_database_suffix(&modules->config,
                               &modules->db->suffix);

    return modules->db;
}

void database_free(struct rena *modules)
{
    struct database *db = modules->db;
    free(db);
    modules->db = NULL;
}
