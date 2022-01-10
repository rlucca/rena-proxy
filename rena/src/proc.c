#include "global.h"
#include "proc.h"

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/signalfd.h>
#include <sys/resource.h>

static void sigset_default(sigset_t *set)
{
    sigemptyset(set);
    sigaddset(set, SIGHUP);
    sigaddset(set, SIGQUIT);
    sigaddset(set, SIGINT);
    sigaddset(set, SIGTERM);
    sigaddset(set, SIGCHLD);
    sigaddset(set, SIGUSR1);
    sigaddset(set, SIGUSR2);
    sigaddset(set, SIGTTIN);
    sigaddset(set, SIGTTOU);
}

int proc_signal_block()
{
    sigset_t set;
    sigset_default(&set);
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
    {
        do_log(LOG_ERROR, "pthread_sigmask: returned -1 (%m)");
        return -1;
    }

    return 0;
}

int proc_create_signalfd()
{
    sigset_t set;
    sigset_default(&set);

    int fd = signalfd(-1, &set, SFD_NONBLOCK|SFD_CLOEXEC);
    if (fd < 0)
    {
        do_log(LOG_ERROR, "signalfd returned -1 (%m)");
        return -1;
    }
    return fd;
}

int proc_receive_signal(int fd)
{
    struct signalfd_siginfo fdsi;
    ssize_t s;

    s = read(fd, &fdsi, sizeof(fdsi));
    if (s != sizeof(fdsi))
    {
        char msg[MAX_STR];
        proc_errno_message(msg, sizeof(msg));
        do_log(LOG_ERROR, "error receiving data from signalfd -- %s", msg);
        return -1;
    }

    return fdsi.ssi_signo;
}

int proc_terminal_signal(int signum)
{
    switch (signum)
    {
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
            return 1;
        default:
            break;
    }

    return 0;
}

int proc_respawn_signal(int signum)
{
    return (signum == SIGCHLD) ? 1 : 0;
}

int proc_raise(int s)
{
    return raise(s);
}

int proc_get_maxfd()
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
    {
        do_log(LOG_DEBUG, "number of files: soft %lu max %lu",
               rl.rlim_cur, rl.rlim_max);
        return rl.rlim_cur;
    }

    return -1;
}

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

int proc_close(int fd)
{
    return close(fd);
}
