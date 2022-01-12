#ifndef DATABASE_H_
#define DATABASE_H_

struct database; // forward
struct rena;

typedef enum {
    DB_NO_PROXY = 0,
    DB_TO_SERVER = 1, // OR FROM CLIENT
    DB_FROM_SERVER = 2, // OR TO CLIENT
    DB_LAST
} db_type_e;

struct database *database_init(struct rena *modules);
void database_free(struct rena *modules);

#endif
