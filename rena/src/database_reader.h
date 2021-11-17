#ifndef DATABASE_READER
#define DATABASE_READER

struct handler_db_reader
{
    int (*is_name_valid)(const char *filename);
    void (*read_line)(struct rena *, char *head, char *tail);
};

int database_reader(const char *dpath,
                    const struct handler_db_reader *h,
                    struct rena *modules);

#endif
