#include "global.h"
#include "database_reader.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

static void database_reader_file(const char *dpath,
                                 const char *filename,
                                 const struct handler_db_reader *h,
                                 struct rena *modules)
{
    FILE *fd = NULL;
    char *fullpath = NULL;
    size_t fullpath_sz = 1;
    char *head = NULL;
    char *tail = NULL;

    if (h && h->is_name_valid && h->is_name_valid(filename) != 0)
    {
        return ;
    }

    fullpath_sz = strlen(dpath) + 1 + strlen(filename) + 1;
    fullpath = malloc(sizeof(char) * fullpath_sz);
    snprintf(fullpath, fullpath_sz, "%s/%s", dpath, filename);

    if ((fd = fopen(fullpath, "r")) == NULL)
    {
        do_log(LOG_ERROR, "Cant read file [%s]", fullpath);
        return ;
    }

    free(fullpath);
    fullpath = NULL;

    while(2 == fscanf(fd, " %ms %m[^\n]", &head, &tail))
    {
        //printf("cmd [%s] <=> [%s]\n", head, tail);
        if (h && h->read_line)
        {
            h->read_line(modules, head, tail);
        }

        free(head);
        free(tail);
    }

    if (!feof(fd))
    {
        do_log(LOG_ERROR, "Cant process all file named [%s/%s]",
               dpath, filename);
    }

    fclose(fd);
}

int database_reader(const char *dpath,
                    const struct handler_db_reader *h,
                    struct rena *modules)
{
    DIR *dir = opendir(dpath);
    struct dirent *dentry = NULL;

    if (dir == NULL)
    {
        do_log(LOG_ERROR, "Error opening directory [%s]", dpath);
        return -1;
    }

    if (h == NULL)
    {
        do_log(LOG_ERROR, "Invalid handler [%p]", h);
        return -2;
    }

    while ((dentry = readdir(dir)) != NULL)
    {
        if (!strcmp(".", dentry->d_name) || !strcmp("..", dentry->d_name))
        {
            continue;
        }

        if (dentry->d_type == DT_DIR)
        {
            do_log(LOG_DEBUG, "Ignoring subdirectory [%s/%s]",
                   dpath, dentry->d_name);
            continue;
        }

        if (dentry->d_type != DT_REG)
        {
            do_log(LOG_DEBUG, "Ignoring file [%s/%s]",
                   dpath, dentry->d_name);
            continue;
        }

        database_reader_file(dpath, dentry->d_name, h, modules);
    }

    return closedir(dir);
}
