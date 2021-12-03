#ifndef RENA_CGI_H_
#define RENA_CGI_H_

struct rena;

int rena_cgi_setup(int, char**, struct rena ** restrict);
void rena_cgi_close(struct rena ** restrict);

#endif
