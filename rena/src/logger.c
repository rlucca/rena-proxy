#include "global.h"
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void syslog_reset(int level, int options,
                         int facility, const char *ident);
static void syslog_message(int level, const char *format, va_list *ptr);
static void stderr_message(int level, const char *format, va_list *ptr);

static struct log_handler_t {
        void (*reset)(int, int, int, const char *);
        void (*message)(int, const char *, va_list *);
        int minimum;
    } log_handler_fnc = { NULL, stderr_message, LOG_DEBUG };


void logger_reset(int level, int options, int facility,
                  const char *ident)
{
    if (log_handler_fnc.reset != NULL)
        log_handler_fnc.reset(level, options, facility, ident);

    logger_message(LOG_DEBUG, "logger reseted");
}

void logger_message(int level, const char *format, ...)
{
    va_list ptr;
    pthread_mutex_lock(&lock);
    va_start(ptr, format);

    if (log_handler_fnc.message != NULL && level <= log_handler_fnc.minimum)
        log_handler_fnc.message(level, format, &ptr);

    va_end(ptr);
    pthread_mutex_unlock(&lock);
}

void logger_reconfigure(void *arg)
{
    struct config_rena *c = (struct config_rena *) arg;
    const char *engine = NULL;
    int logging[3];

    if (c == NULL)
    {
        do_log(LOG_DEBUG, "invalid parm [%p]", arg);
        return ;
    }

    pthread_mutex_lock(&lock);
    config_get_logging_engine(&c, &engine);
    config_get_logging_minimum(&c, &logging[0]);
    config_get_logging_options(&c, &logging[1]);
    config_get_logging_facility(&c, &logging[2]);
    log_handler_fnc.minimum = logging[0];
    if (strcmp("syslog", engine) == 0)
    {
        log_handler_fnc.reset = syslog_reset;
        log_handler_fnc.message = syslog_message;
    } else {
        log_handler_fnc.reset = NULL;
        log_handler_fnc.message = stderr_message;
    }
    pthread_mutex_unlock(&lock);

    logger_reset(logging[0], logging[1], logging[2], NULL);
    do_log(LOG_DEBUG, "done");
}

static void syslog_reset(int level, int options,
                         int facility, const char *ident)
{
    closelog();
    openlog(ident, options, facility);
    setlogmask(LOG_UPTO(level));
}

static void syslog_message(int level, const char *format, va_list *ptr)
{
    vsyslog(level, format, *ptr);
}

static void stderr_message(int level, const char *format, va_list *ptr)
{
    fprintf(stderr, "LVL %d - ", level);
    vfprintf(stderr, format, *ptr);
    fprintf(stderr, "\n");
}
