#include "global.h"
#include "proc.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
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
    sigaddset(set, SIGPIPE);
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
        text_t msg;
        int code = proc_errno_message(&msg);
        if (code != EAGAIN && code != EWOULDBLOCK && code != EINTR)
            do_log(LOG_ERROR,
                   "error receiving data from signalfd -- %s",
                   msg.text);
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

int proc_errno_message(text_t *t)
{
    int erro = errno;
    int text_len = sizeof(t->text);
    t->size = snprintf(t->text, text_len, "(%d) ", erro);
    if (t->size >= text_len) return erro;
    if (strerror_r(erro, t->text + t->size, text_len - t->size) < 0)
    {
        t->size += snprintf(t->text + t->size, text_len - t->size, "unknown");
    }
    t->text[text_len - 1] = '\0';
    return erro;
}

int proc_close(int fd)
{
    return close(fd);
}

int proc_valid_fd(int fd)
{
    int ret = fcntl(fd, F_GETFD);
    if (ret < 0)
    {
        text_t msg;
        int E = proc_errno_message(&msg);
        if (E == EBADF)
            return 0;
    }
    return 1;
}

int proc_limit_fds()
{
    #define MAX 65536
    struct rlimit init;
    if (getrlimit(RLIMIT_NOFILE, &init) != 0)
    {
        init.rlim_max = MAX;
    } else {
        if (init.rlim_max <= MAX)
            init.rlim_max = MAX;
    }
    #undef MAX
    init.rlim_cur = init.rlim_max;
    return setrlimit(RLIMIT_NOFILE, &init);
}
