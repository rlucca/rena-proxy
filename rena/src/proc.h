#ifndef PROC_H_
#define PROC_H_

int proc_signal_block();
int proc_create_signalfd();
int proc_raise(int);
int proc_get_maxfd();
int proc_errno_message(text_t *);
int proc_receive_signal(int fd);
int proc_terminal_signal(int signum);
int proc_respawn_signal(int signum);
int proc_close(int fd);
int proc_valid_fd(int fd);
int proc_limit_fds();

#endif
