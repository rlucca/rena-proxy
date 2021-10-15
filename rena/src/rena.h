#ifndef RENA_H_
#define RENA_H_

struct rena_t;

int rena_setup(int, char**, struct rena_t ** restrict);
int rena_run(struct rena_t ** restrict);

#endif
