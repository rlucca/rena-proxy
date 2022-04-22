
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#include "global.h"
#include "logger.h"

CHEAT_DECLARE(
       int openlog_called = 0;
       int closelog_called = 0;
       int vsyslog_called = 0;
       int setlogmask_called = 0;
       int setlogmask_mask = 0;
       void openlog(const char *ident, int option, int facility)
        {
            openlog_called++;
        }
       void closelog(void)
        {
            closelog_called++;
        }
       void vsyslog(int priority, const char *format, va_list ap)
        {
            vsyslog_called++;
        }
       int setlogmask(int mask)
        {
            setlogmask_mask = mask;
            setlogmask_called++;
            return 0;
        }

        struct config_rena // copied from config.c
        {
            char certificate_file[MAX_FILENAME];
            char certificate_key[MAX_FILENAME];
            char database_directory[MAX_FILENAME];
            char database_suffix[MAX_STR];
            char server_bind[MAX_STR];
            int server_port_http;
            int server_port_https;
            int logging_facility;
            int logging_options;
            int logging_minimum;
            int pool_minimum;
            int pool_maximum;
            int pool_reap_time;
            float pool_addictive;
            char parser_analyze_mime[MAX_STR];
            char parser_ignore_mime[MAX_STR];
            char parser_analyze_accept[MAX_STR];
            char parser_ignore_accept[MAX_STR];
            char logging_engine[MAX_STR];
        };
)

CHEAT_SET_UP(
        struct config_rena obj;
        const char *s = "syslog";
        memcpy(obj.logging_engine, s, strlen(s) + 1);
        logger_reconfigure(&obj);
        openlog_called = closelog_called = 0;
        vsyslog_called = setlogmask_called = 0;
        setlogmask_mask = 0;
    )

CHEAT_TEST(call_logger_reset,
    logger_reset(3, 0, 0, NULL);
    cheat_assert(openlog_called == 1);
    cheat_assert(closelog_called == 1);
    cheat_assert(setlogmask_called == 1);
    cheat_assert(setlogmask_mask == ((1 << 4) - 1));
)

CHEAT_TEST(call_logger_message,
    logger_message(0, "xxx");
    cheat_assert(vsyslog_called == 1);
)
