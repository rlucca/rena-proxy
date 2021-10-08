#ifndef LOGGER_H_
#define LOGGER_H_

// LOG LEVEL {
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERROR   3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */
// }

void logger_reset(int level, int options, int facility,
                  const char *ident);
void logger_message(int level, const char *format, ...)
                       __attribute__ ((__format__ (__printf__, 2, 3)));

#define do_log(level, fmt, ...) \
    logger_message(level, "%s:%s:%d -- " fmt, \
                   __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif
