#ifndef PROC_H_
#define PROC_H_

int proc_signal_block();
int proc_create_signalfd();
int proc_raise(int);
int proc_get_maxfd();
void proc_errno_message(char *buf, size_t buf_len);
int proc_receive_signal(int fd);
int proc_terminal_signal(int signum);

#endif
