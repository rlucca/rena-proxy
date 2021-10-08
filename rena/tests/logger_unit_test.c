
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

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
