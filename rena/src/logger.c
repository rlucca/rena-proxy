#include "global.h"
#include <stdarg.h>
#include <syslog.h>


void logger_reset(int level, int options, int facility,
                  const char *ident)
{
    closelog();
    openlog(ident, options, facility);
    setlogmask(LOG_UPTO(level));
}

void logger_message(int level, const char *format, ...)
{
    va_list ptr;
    va_start(ptr, format);
    vsyslog(level, format, ptr);
    va_end(ptr);
}

