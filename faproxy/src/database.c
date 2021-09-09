#include "globals.h"
#include "database_internal.h"
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

//#undef DEBUG_MODE_ON
static tree_node_t *main_db[LAST_N_TREE] = { NULL, NULL, NULL };

static void insert_never(const char *domain)
{
    if (domain == NULL)
        return ;

    main_db[NEVER_PROXY_TREE] = tree_insert(
                                main_db[NEVER_PROXY_TREE],
                                domain, strlen(domain), domain,
                                "");
}

static void insert_rewrite(const char *server, const char *client)
{
    char expanded_client[MAX_STR];

    snprintf(expanded_client, sizeof(expanded_client),
             "%s%s", client, events.server_suffix);
    main_db[RENAME_FROM_SERVER] = tree_insert(
                                main_db[RENAME_FROM_SERVER],
                                server, strlen(server), client,
                                events.server_suffix);
    main_db[RENAME_FROM_CLIENT] = tree_insert(
                                main_db[RENAME_FROM_CLIENT],
                                expanded_client, strlen(expanded_client),
                                server, "");
}

static void expand_database_from(const char *filename, const char *directory)
{
    char full_path[MAX_STR];
    char line[MAX_STR];
    char left[MAX_STR];
    char right[MAX_STR];
    FILE *fd = NULL;

    snprintf(full_path, sizeof(full_path), "%s/%s", directory, filename);

    if (access(full_path, R_OK) != 0)
    {
        fprintf(stderr, "no read permission on [%s]\n", full_path);
        return ;
    }

    fd = fopen(full_path, "r");

    while (fscanf(fd, " %[^\n]", line) >= 0)
    {
        if (sscanf(line, " proxy_domain_rename %s %s\n", left, right) == 2)
        {
            insert_rewrite(left, right);
        } else if (sscanf(line, " no_proxy %s\n", left) == 1)
        {
            insert_never(left);
        }
        
        // DEBUG("%s:%s -=- %s %s", full_path, line, left, right);
    }
}

void database_set_up(const char *directory)
{
        DIR *dirs = opendir(directory);

        if (dirs == NULL) {
            fprintf(stderr, "Problem reading directory: %s\n", directory);
            abort();
            return ;
        }

        struct dirent *ents = NULL;
        do {
            errno = 0;
            ents = readdir(dirs);
            
            if (ents)
            {
                if (ents->d_name[0] != '.' || ents->d_name[0] != '_')
                    expand_database_from(ents->d_name, directory);
                else
                    DEBUG("Ignorando arquivo [%s/%s]...", directory, ents->d_name);
            }

        } while (ents != NULL);
        
        closedir(dirs);

#ifdef DEBUG_MODE_ON
        char phrase[MAX_STR];
        memset(phrase, 0, MAX_STR);
        DEBUG("--- NO_PROXY");
        tree_dump(di_get_tree(NEVER_PROXY_TREE), phrase, 0);
        DEBUG("--- TO SERVER");
        memset(phrase, 0, MAX_STR);
        tree_dump(di_get_tree(RENAME_FROM_SERVER), phrase, 0);
        DEBUG("--- TO CLIENT");
        memset(phrase, 0, MAX_STR);
        tree_dump(di_get_tree(RENAME_FROM_CLIENT), phrase, 0);
#endif
}

tree_node_t *di_get_tree(size_t pos)
{
    const size_t max = sizeof(main_db) / sizeof(*main_db);
    if (pos >= max || pos < 0)
    {
        DEBUG("calling abort out of array");
        abort();
        return NULL;
    }

    return main_db[pos];
}
