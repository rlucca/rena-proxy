#include "global.h"
#include "user_list.h"
#include "tree.h"

#include <stdlib.h>
#include <string.h>


static int read_lines(FILE *fd, handler_t *h, struct rena *ud)
{
    char *tmp[2];
    int itmp = 0;
    while (fscanf(fd, " %ms", &tmp[itmp]) == 1)
    {
        itmp = (itmp + 1) % 2;

        if (itmp != 0 || h == NULL)
            continue;

        h(ud, tmp[0], tmp[1]);
        free(tmp[0]);
        free(tmp[1]);
    }

    return itmp;
}

int database_user_list_reader(const char *filename,
                              handler_t *h, struct rena *user_data)
{
    FILE *fd = fopen(filename, "r");

    if (!fd)
    {
        do_log(LOG_ERROR, "Error reading filename [%s]", filename);
        return 1;
    }

    if (read_lines(fd, h, user_data))
    {
        do_log(LOG_ERROR,
                "Did your file [%s] has 'user <space> password' as lines?",
                filename);
        fclose(fd);
        return 2;
    }

    fclose(fd);
    return 0;
}

int database_user_list_verify(void *subtree,
                              const char *userpass[2],
                              const long unsigned int userpass_len[2])
{
    tree_node_t *root = (tree_node_t *) subtree;
    const char *username = userpass[0];
    tree_node_t *tmp = tree_get_sibling(root, username[0]);

    for (int I = 1; tmp != NULL && I < userpass_len[0]; I++)
        tmp = tree_get_child(tmp, username[I]);

    if (tmp && tmp->adapted && userpass[1])
    {
        int length = strnlen(tmp->adapted, MAX_STR);
        if (length == userpass_len[1]
                && !strncmp(tmp->adapted, userpass[1], length))
        {
            return 0;
        }
    }

    return 1;
}
