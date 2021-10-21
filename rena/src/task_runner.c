
#include "global.h"
#include "task_runner.h"

#include <unistd.h>

void *task_runner(void *arg)
{
    struct rena *rena = (struct rena *) arg;
    for (;rena->forced_exit == 0;)
    {
        pause();
    }

    return NULL;
}
