#include "globals.h"
#include "database.h"
#include <sys/time.h>
#include <unistd.h>

static int exitp = 0;

static void set_exit(int sig)
{
    exitp = 1;
}

static double timer_in_seconds(struct timeval *start,
                               struct timeval *stop)
{
    double ret = 0.0;
    struct timeval result;
    long miliseconds;

    timerclear(&result);
    timersub(stop, start, &result);

    miliseconds = result.tv_sec * 1000.0;
    miliseconds += (result.tv_usec / 1000.0);

    ret = miliseconds / 1000.0;
    return ret;
}

static int read_file_in(const char *filename, char **buffer, size_t *buffer_len)
{
    FILE *fd = NULL;
    long file_size = 0;
    long read_ret = 0;

    if (filename == NULL || *filename == '\0'
        || buffer == NULL || *buffer != NULL || buffer_len == NULL)
    {
        fprintf(stderr, "incorrect parms!\n");
        return -1;
    }

    fd = fopen(filename, "rb");
    if (fd == NULL)
    {
        fprintf(stderr, "error opening [%s]\n", filename);
        return -2;
    }
    fseek(fd, 0L, SEEK_END);
    file_size = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    DEBUG("file [%s] = %ld", filename, file_size);
    *buffer = (char *) malloc(sizeof(char) * (1 + file_size));
    read_ret = fread(*buffer, sizeof(char), file_size, fd);
    (*buffer)[file_size] = 0;
    
    if (read_ret == file_size) {
        *buffer_len = read_ret;
        DEBUG("read [%s] all to buffer", filename);
        return 0;
    }

    DEBUG("read [%s] all to buffer failed", filename);
    return -3;
}

static char *
re_alloc(char *to_change, size_t n_elems)
{
    char *old = to_change;
    char *aux = realloc(to_change, n_elems);
    if (aux == NULL) // maybe the old support us
        return old;
    return aux;
}

static void do_the_test(char *buffer_input,
                        size_t buffer_len,
                        char **output,
                        size_t *output_len)
{
    const int client_side = -1;
    char *expanded = NULL;
    size_t consumed = 0;
    void *handler;
    char *holding = NULL;
    size_t holding_size = 0;
    size_t holding_reserve = 0;

    if (output == NULL || buffer_input == NULL)
    {
        fprintf(stderr, "incorrect parms\n");
        return ;
    }

    handler = database_transform_handler(client_side);
    if (handler == NULL)
    {
        fprintf(stderr, "handler nulled [%p]\n", handler);
        return ;
    }

    DEBUG("transform [%p] size [%lu]", buffer_input, buffer_len);
    for (unsigned u = 0; u < buffer_len; u++)
    {
        int res = database_transform_lookup(
                                        &handler,
                                        buffer_input[u],
                                        0, &expanded, &consumed);

        if (res == NOT_HOLD)
        {
            if (holding_size)
            {
                *output = re_alloc(*output, sizeof(char) * (holding_size + output_len[0]));
                for (size_t ss = 0; ss < holding_size; ss++, (*output_len) += 1)
                {
                    (*output)[*output_len] = holding[ss];
                }

                //DEBUG("holding write [%.*s] plus %c", (int) holding_size, holding, buffer_input[u]);
                holding_size = 0;
            }
            *output = re_alloc(*output, sizeof(char) * (1 + output_len[0]));
            (*output)[*output_len] = buffer_input[u];
            *output_len += 1;
        } else if (res == FEED_ME_AND_HOLD)
        {
            if ((1 + holding_size) >= holding_reserve)
            {
                holding_reserve += 1024;
                holding = re_alloc(holding, sizeof(char) * (1 + holding_reserve));
            }
            holding[holding_size] = buffer_input[u];
            holding_size += 1;
        } else if (res == TRANSFORM_OK)
        {
            size_t len = strlen(expanded);

            if (holding_size > 0)
            { 
                //DEBUG("holding ignored [%.*s] plus %c || %lu / %lu",
                //      (int) holding_size, holding, buffer_input[u],
                //      consumed - 1, holding_size);
                holding_size -= consumed - 1;

                *output = re_alloc(*output, sizeof(char) * (holding_size + output_len[0]));
                for (size_t ss = 0; ss < holding_size; ss++, (*output_len) += 1)
                {
                    (*output)[*output_len] = holding[ss];
                }
            }

            holding_size = 0;

            if (len > 0)
            {
                *output = re_alloc(*output, sizeof(char) * (len + output_len[0]));
                for (size_t ss = 0; ss < len; ss++, (*output_len) += 1)
                {
                    (*output)[*output_len] = expanded[ss];
                }
            }
        }
    }

    if (holding_size > 0)
    { 
        *output = re_alloc(*output, sizeof(char) * (holding_size + output_len[0]));
        for (size_t ss = 0; ss < holding_size; ss++, (*output_len) += 1)
        {
            (*output)[*output_len] = holding[ss];
        }

        holding_size = 0;
    }

    DEBUG("cleaning stuff...");
    database_transform_cleanup(&handler);
}

static void timed_test(const char *input_file,
                       const char *expected_file)
{
    char *input = NULL;
    size_t input_len = -1;
    char *output = NULL;
    size_t output_len = 0;
    char *expected = NULL;
    size_t expected_len = -1;
    struct timeval timestart, timeend;
    DEBUG("------------------------------------------------------");
    DEBUG("input [%s] expected [%s]",
          input_file, expected_file);

    if (read_file_in(input_file, &input, &input_len) != 0) {
        return ;
    }

    if (read_file_in(expected_file, &expected, &expected_len) != 0) {
        return ;
    }

    DEBUG("all done! Starting test...");
    gettimeofday(&timestart, NULL);

    do_the_test(input, input_len, &output, &output_len);

    gettimeofday(&timeend, NULL);
    fprintf(stdout, "Finished in %.3lf...\n",
            timer_in_seconds(&timestart, &timeend));

    DEBUG("Starting to compare with expected_file...");
    if (output == NULL || strncmp(output, expected, expected_len) != 0)
        fprintf(stderr, "Output doesnt match! %p %p\n", output, expected);

    DEBUG("expected [%u] output [%u]", expected[expected_len - 1], output[output_len - 1]);

    char filename_tmp[1024];
    const char *basename = strrchr(expected_file, '/');
    if (basename == NULL) basename = expected_file;
    snprintf(filename_tmp, sizeof(filename_tmp), "/tmp/%s", basename);
    FILE *fd = fopen(filename_tmp, "w");
    if (output)
        fwrite(output, sizeof(char), output_len, fd);
    fclose(fd);

    DEBUG("%lu :: %lu == %lu", input_len, expected_len, output_len);

    free(input);
    free(expected);
    free(output);
}


int main(int argc, char** argv)
{
    char dbs_file[MAX_STR];
    const char *test_files[] = {
            //"tests/files/transform.html",
            //"tests/files/transform2.html",
            "tests/files/busca-periodicos_1.html",
            "tests/files/busca-periodicos_2.html",
            "tests/files/jwatch.html",
            NULL
        };
    const char *result_files[] = {
            //"tests/files/transform_result.html",
            //"tests/files/transform2_result.html",
            "tests/files/busca-periodicos_1_result.html",
            "tests/files/busca-periodicos_2_result.html",
            "tests/files/jwatch_result.html",
            NULL
        };

    snprintf(dbs_file, sizeof(dbs_file), "/etc/fAproxy/dbs");
    snprintf(events.server_suffix, sizeof(events.server_suffix),
             "%s", ".ezN.periodicos.capes.gov.br");

    // prepare database
    database_set_up(dbs_file);

    signal(SIGALRM, set_exit);
    alarm(640); // mais ou menos mesmo tempo do python

    while (exitp == 0)
        for (unsigned u = 0; test_files[u] != NULL; u++)
        {
            timed_test(test_files[u], result_files[u]);
        }

    return 0;
}
