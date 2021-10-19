#include "global.h"
#include <stdarg.h>
#include <syslog.h>


void logger_reset(int level, int options, int facility,
                  const char *ident)
{
    closelog();
    openlog(ident, options, facility);
    setlogmask(LOG_UPTO(level));

    logger_message(LOG_DEBUG, "logger reseted");
}

void logger_message(int level, const char *format, ...)
{
    va_list ptr;
    va_start(ptr, format);
    vsyslog(level, format, ptr);
    va_end(ptr);
}

void logger_reconfigure(void *arg)
{
    struct config_rena *c = (struct config_rena *) arg;
    int logging[3];

    if (c == NULL)
    {
        do_log(LOG_DEBUG, "invalid parm [%p]", arg);
        return ;
    }

    config_get_logging_minimum(&c, &logging[0]);
    config_get_logging_options(&c, &logging[1]);
    config_get_logging_facility(&c, &logging[2]);
    logger_reset(logging[0], logging[1], logging[2], NULL);
    do_log(LOG_DEBUG, "done");
}
