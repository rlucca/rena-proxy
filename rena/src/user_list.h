#ifndef _USER_LIST_H_
#define _USER_LIST_H_

struct rena;

typedef void (handler_t)(struct rena *, const char *, const char *);
int database_user_list_reader(const char *filename,
                              handler_t *, struct rena *);
int database_user_list_verify(void *subtree, const char *userpass[2],
                              size_t userpass_len[2]);

#endif
