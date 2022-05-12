#include "global.h"

#include "task_manager.h"
#include "database.h"
#include "clients.h"
#include "server.h"
#include "proc.h"

#include <stdlib.h>
#include <unistd.h>
#include <valgrind.h>

static void rena_usage(char **argv)
{
    fprintf(stderr,
            "Usage: %s [-c config_file.ini] [-d] [-v]\n",
            argv[0]);
}

static void rena_version(void)
{
    fprintf(stderr,
            "Commit ID: %s\n",
            RENA_ID);
    fprintf(stderr,
            "Commit Title: %s\n",
            RENA_TITLE);
    fprintf(stderr,
            "Commit Date: %s\n\n",
            RENA_DATE);
    fprintf(stderr,
            "Compiled Date: %s %s\n",
            __DATE__, __TIME__);
}

static int rena_process_args(int argc, char **argv,
                             struct rena **modules)
{
    char filename[MAX_FILENAME];
    int opt;
    int version = 0;

    snprintf(filename, sizeof(filename),
             "/etc/rena/rena.ini");

    while ((opt = getopt(argc, argv, "c:dv")) != -1)
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
            case 'v':
                version = 1;
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

    if (version)
    {
        rena_version();
        return -4;
    }

    if (access(filename, R_OK) != 0)
    {
        fprintf(stderr, "%s: failed to read -- %s\n",
                        argv[0], filename);
        return -3;
    }

    return config_load(&(*modules)->config, filename);
}

static void rena_destroy(struct rena **modules)
{
    task_manager_destroy(*modules);
    server_destroy(*modules);
    clients_destroy(&((*modules)->clients));
    database_free(*modules);
    config_free(&((*modules)->config));
    free(*modules);
}


int rena_setup(int argc, char **argv,
                struct rena **modules)
{
    logger_reset(LOG_DEBUG, 0x2b, 182, NULL);
    *modules = calloc(1, sizeof(struct rena));

    int ret = rena_process_args(argc, argv, modules);
    if (ret != 0)
    {
        rena_destroy(modules);
        return ret;
    }

    logger_reconfigure((*modules)->config);

    if (RUNNING_ON_VALGRIND == 0 && proc_limit_fds() != 0)
    {
        fprintf(stderr, "error setting maximal fds: %m\n");
        rena_destroy(modules);
        return -9;
    }
    (void) proc_get_maxfd();

    if (database_init(*modules) == NULL)
    {
        rena_destroy(modules);
        return -6;
    }

    if (((*modules)->clients = clients_init()) == NULL)
    {
        rena_destroy(modules);
        return -8;
    }

    if (server_init(*modules) == NULL)
    {
        rena_destroy(modules);
        return -7;
    }

    if (task_manager_init(*modules) == NULL)
    {
        rena_destroy(modules);
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
        text_t buf;
        proc_errno_message(&buf);
        do_log(LOG_ERROR, "Error during fork: %s", buf.text);
    }

    task_manager_run(*modules);
    rena_destroy(modules);
    return 0;
}
