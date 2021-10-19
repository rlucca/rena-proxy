#ifndef GLOBAL_H_
#define GLOBAL_H_

#define MAX_FILENAME 5120
#define MAX_STR 1024

#include "logger.h"
#include "config.h"
#include "rena.h"

#include <stdio.h>

struct task_manager;
struct server;

struct rena {
    struct config_rena *config;
    struct task_manager *tm;
    struct server *server;
    int daemonize;
    int exit;
};

#endif
