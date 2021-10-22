#include "global.h"
#include "proc.h"

#include <string.h>
#include <errno.h>


void proc_errno_message(char *buf, size_t buf_len)
{
    int erro = errno;
    size_t code_len = 0;
    snprintf(buf, buf_len, "(%d) ", erro);
    code_len = strnlen(buf, buf_len);
    if (code_len >= buf_len) return ;
    if (strerror_r(erro, buf + code_len, buf_len) < 0)
    {
        snprintf(buf + code_len, buf_len, "unknown");
    }
    buf[buf_len - 1] = '\0';
}
