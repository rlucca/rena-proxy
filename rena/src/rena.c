#include "global.h"

#include "task_manager.h"
#include "database.h"
#include "server.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void rena_usage(char **argv)
{
    fprintf(stderr,
            "Usage: %s [-c config_file.ini] [-d]\n",
            argv[0]);
}

static int rena_process_args(int argc, char **argv,
                             struct rena **modules)
{
    char filename[MAX_FILENAME];
    int opt;

    snprintf(filename, sizeof(filename),
             "/etc/rena/rena.ini");

    while ((opt = getopt(argc, argv, "c:d")) != -1)
    {
        switch (opt)
        {
            case 'c':
                snprintf(filename, sizeof(filename),
                         "%s", optarg);
                break;
            case 'd':
                (*modules)->daemonize = 1;
                break;

            default: /* '?' */
                rena_usage(argv);
                return -1;
        }
    }

    if (optind != argc)
    {
        fprintf(stderr, "%s: unexpected argument at end\n",
                        argv[0]);
        rena_usage(argv);
        return -2;
    }

    if (access(filename, R_OK) != 0)
    {
        fprintf(stderr, "%s: failed to read -- %s\n",
                        argv[0], filename);
        return -3;
    }

    return config_load(&(*modules)->config, filename);
}


int rena_setup(int argc, char **argv,
                struct rena **modules)
{
    logger_reset(LOG_DEBUG, 0x2b, 182, NULL);
    *modules = calloc(1, sizeof(struct rena));

    int ret = rena_process_args(argc, argv, modules);
    if (ret != 0)
    {
        return ret;
    }

    logger_reconfigure((*modules)->config);

    if (database_init(*modules) == NULL)
    {
        return -6;
    }

    if (server_init(*modules) == NULL)
    {
        return -7;
    }

    if (task_manager_init(*modules) == NULL)
    {
        return -5;
    }

    return 0;
}

int rena_run(struct rena **modules)
{
    if ((*modules)->daemonize == 0)
    {
        do_log(LOG_DEBUG, "Not going to background");
    } else if (daemon(0, 0) != 0)
    {
        do_log(LOG_ERROR, "Error during fork [%d]", errno);
    }

    task_manager_run(*modules);
    task_manager_destroy(*modules);
    server_destroy(*modules);
    database_free(*modules);
    config_free(&((*modules)->config));
    return 0;
}
