#ifndef FORMATTER_H_
#define FORMATTER_H_

struct formatter;

struct formatter_userdata
{
    char *out;
    size_t out_sz;
    struct rena *rena;
    client_position_t *client;
};

int formatter_create_handler(struct formatter **inout, const char *, size_t );
int formatter_destroy_handler(struct formatter **inout);
int formatter_evaluate(struct formatter **inout, struct formatter_userdata *);

#endif
