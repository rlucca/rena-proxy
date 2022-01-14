#ifndef DATABASE_H_
#define DATABASE_H_

struct database_object; // forward
struct database; // forward
struct rena;

typedef enum {
    DB_NO_PROXY = 0,
    DB_TO_SERVER = 1, // OR FROM CLIENT
    DB_FROM_SERVER = 2, // OR TO CLIENT
    DB_LAST
} db_type_e;

typedef enum {
    DBI_NOT_HOLD = 0,
    DBI_FEED_ME = 1,
    DBI_TRANSFORMATION_FOUND = 2
} di_output_e;

struct database *database_init(struct rena *modules);
void database_free(struct rena *modules);
struct database_object *database_instance_create(struct rena *,
                                                 int is_victim);
di_output_e database_instance_lookup(struct database_object *,
                                     char,
                                     const char ** const,
                                     int * const);
void database_instance_get_holding(struct database_object *d,
                                   const char ** const o,
                                   int * const olen);
void database_instance_dump(struct database_object *d);
void database_instance_destroy(struct database_object **);

#endif
