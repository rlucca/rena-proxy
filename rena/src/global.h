#ifndef GLOBAL_H_
#define GLOBAL_H_

#define MAX_FILENAME 5120
#define MAX_STR 1024

#include "logger.h"
#include "config.h"
#include "rena.h"

struct rena_t {
    struct config_rena_t *config;
    int daemonize;
};

#endif
