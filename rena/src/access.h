#ifndef LOG_ACCESS_H_
#define LOG_ACCESS_H_

struct client_position;
struct rena;

int log_access_start(struct rena *rena);
void log_access_stop(void);

int log_access(struct client_position *client);

#endif
