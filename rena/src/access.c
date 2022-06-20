#include "global.h"
#include "formatter.h"
#include "config.h"
#include "access.h"

#include <string.h>
#include <pthread.h>
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static const char *filename = NULL;
static struct formatter *format = NULL;


static int do_log_access(const char *line, int line_len)
{
    FILE *fd = fopen(filename, "a");
    if (fd == NULL)
        return -2;

    if (line != NULL)
    {
        fprintf(fd, "%.*s\n", line_len, line);
    }

    fclose(fd);
    return 0;
}

int log_access_start(struct rena *rena)
{
    const char *tmp = NULL;
    int ret = -1;
    config_get_logging_access_file(&rena->config, &filename);
    config_get_logging_access_format(&rena->config, &tmp);
    if (tmp && filename)
    {
        pthread_mutex_lock(&lock);
        ret = formatter_create_handler(&format, tmp, strnlen(tmp, MAX_STR));
        if (ret == 0)
        {
            ret = do_log_access(NULL, 0);
        }
        pthread_mutex_unlock(&lock);
    }
    return ret;
}

void log_access_stop(void)
{
    pthread_mutex_lock(&lock);
    formatter_destroy_handler(&format);
    pthread_mutex_unlock(&lock);
}

int log_access(struct client_position *c)
{
    char line[8 * MAX_STR];
    struct formatter_userdata mine = { line, 8 * MAX_STR, 0, c };
    int ret = -1;

    pthread_mutex_lock(&lock);
    ret = formatter_evaluate(&format, &mine);
    if (!ret)
    {
        ret = do_log_access(line, (int) mine.out_len);
    }
    pthread_mutex_unlock(&lock);
    return ret;
}
