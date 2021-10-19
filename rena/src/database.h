#ifndef DATABASE_H_
#define DATABASE_H_

struct database; // forward
struct rena;

struct database *database_init(struct rena *modules);
void database_free(struct rena *modules);

#endif
