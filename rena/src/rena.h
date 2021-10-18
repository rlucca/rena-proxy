#ifndef RENA_H_
#define RENA_H_

struct rena;

int rena_setup(int, char**, struct rena ** restrict);
int rena_run(struct rena ** restrict);

#endif
