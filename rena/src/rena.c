#include "global.h"

#include <stdio.h>
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
    int logging[3];

    logger_reset(LOG_DEBUG, 0x2b, 182, NULL);
    *modules = calloc(1, sizeof(struct rena));

    int ret = rena_process_args(argc, argv, modules);
    if (ret != 0)
    {
        return ret;
    }

    config_get_logging_minimum(&(*modules)->config, &logging[0]);
    config_get_logging_options(&(*modules)->config, &logging[1]);
    config_get_logging_facility(&(*modules)->config, &logging[2]);
    logger_reset(logging[0], logging[1], logging[2], NULL);
    logger_message(LOG_DEBUG, "Configuration loaded");

    // TODO

    return -1;
}

int rena_run(struct rena **modules)
{
    if ((*modules)->daemonize == 0)
    {
        logger_message(LOG_DEBUG, "Not going to background");
    } else if (daemon(0, 0) != 0)
    {
        logger_message(LOG_ERROR, "Error during fork [%d]", errno);
    }

    // TODO

    return -1;
}
