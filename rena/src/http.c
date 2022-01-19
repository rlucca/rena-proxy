
#include "global.h"
#include "http.h"

#include <stdlib.h>

void http_destroy(void *handler)
{
    if (handler == NULL)
        return ;

    free(handler);
}

int http_pull(struct rena *rena, client_position_t *client, int fd)
{
    return -1;
}

int http_push(struct rena *rena, client_position_t *client, int fd)
{
    return -1;
}

int http_evaluate(client_position_t *client)
{
    return -1;
}
