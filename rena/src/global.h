#ifndef GLOBAL_H_
#define GLOBAL_H_

#define MAX_FILENAME 5120

#include "text.h"
#include "logger.h"
#include "config.h"
#include "rena.h"

#include <stdio.h>

struct task_manager;
struct database;
struct server;
struct clients;

struct rena {
    struct config_rena *config;
    struct task_manager *tm;
    struct server *server;
    struct database *db;
    struct clients *clients;
    int daemonize;
    volatile int forced_exit;
};

#endif
