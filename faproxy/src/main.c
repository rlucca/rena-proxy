#include "globals.h"
#include "database.h"
#include "ssl.h"
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>


static int create_listener(int port)
{
    int sd = prepare_socket(port);
    if ( listen(sd, 32) != 0 )
    {
        perror("Can't go to listening state!");
        close(sd);
        return -1;
    }

    // deixamos fora do prepare socket para quando conectarmos
    // como cliente e chamarmos a funcao ela ser bloqueante para
    // quando jogar nas leituras/escrita ser nao bloqueante
    setnonblock(sd);

    return sd;
}

static void do_nothing(struct ev_loop *loop, struct ev_async *w, int revents)
{
    /* nothing to do, just wake up */
}

static void signal_f (struct ev_loop *loop, struct ev_signal *w, int revents)
{
    switch(w->signum)
    {
        case SIGINT:
            DEBUG("Received SIGINT, shutting down");
            ev_unloop (loop, EVUNLOOP_ALL);
            break;
        case SIGTERM:
            DEBUG("Received SIGTERM, shutting down");
            ev_unloop (loop, EVUNLOOP_ALL);
            break;
        default:
            DEBUG("Received Unknown Signal: %d", w->signum);
    }
}

DECLARE_ENQUEUE_ACTION(accept_http_f, ACTION_ACCEPT, 0)
DECLARE_ENQUEUE_ACTION(accept_secure_f, ACTION_ACCEPT, 's')

int main(int argc, char** argv)
{
    char bind_addr[MAX_STR];
    char cert_file[MAX_STR];
    char key_file[MAX_STR];
    char dbs_file[MAX_STR];
    struct in_addr bin_addr;
    char c;
    size_t i;

    snprintf(bind_addr, sizeof(bind_addr), "127.0.0.1");
    snprintf(dbs_file, sizeof(dbs_file), "/etc/fAproxy/dbs");
    memset(events.server_suffix, 0, sizeof(events.server_suffix));
    snprintf(cert_file, sizeof(cert_file), "certs.pem");
    snprintf(key_file, sizeof(key_file), "keys.pem");

    while ((c = getopt (argc, argv, "b:c:k:d:t:s:h")) != -1)
    {
        switch (c)
        {
            case 'b':
                snprintf(bind_addr, sizeof(bind_addr), "%s", optarg);
                break;
            case 'c':
                snprintf(cert_file, sizeof(cert_file), "%s", optarg);
                break;
            case 'k':
                snprintf(key_file, sizeof(key_file), "%s", optarg);
                break;
            case 'd':
                snprintf(dbs_file, sizeof(dbs_file), "%s", optarg);
                break;
            case 's':
                snprintf(events.server_suffix, sizeof(events.server_suffix), "%s", optarg);
                break;
            case 't':
                sscanf(optarg, "%zu", &n_threads);
                break;
            case 'h':
            default:
                fprintf(stderr, "fAproxy Usage:\n"
                                "-b 127.0.0.1                     - bind to address 127.0.0.1\n"
                                "                                   port used: %d and %d\n"
                                "-d /etc/fAproxy/dbs              - database directory\n"
                                "-c certs.pem                     - cert file\n"
                                "-k keys.pem                      - private key file to cert file\n"
                                "-s .fa1.periodicos.capes.gov.br  - server suffix\n"
                                "-t 4                             - start 4 threads\n",
                        HTTP_PORT, HTTPS_PORT);
                exit(-1);
        }
    }

    // prepare database
    database_set_up(dbs_file);

    if (inet_pton(AF_INET, bind_addr, &bin_addr) == 0)
    {
        fprintf(stderr, "failed to convert [%s] to binary address to bind!\n", bind_addr);
        return 1;
    }

    events.bind_addr = &bin_addr;

    if (events.server_suffix[0] == '\0' || events.server_suffix[0] != '.')
    {
        fprintf(stderr, "failed to start suffix is null or not started "
                        "with period: [%s]\n", events.server_suffix);
        return 1;
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    setnonblock(STDIN_FILENO);

    // start queue
    memset(events.jobs, 0, sizeof(events.jobs));
    queue_t allJobs = QUEUE_INITIALIZER(events.jobs);
    events.job_queue = &allJobs;

    // start libev
    events.main_loop = ev_loop_new(0);
    //ev_set_userdata(events.main_loop, NULL); // LATER
    
    if (events.main_loop == NULL)
    {
        fprintf(stderr, "Could not start main loop\n");
        return 1;
    }

    ev_async_init(&events.wake_up, do_nothing);
    ev_async_start(events.main_loop, &events.wake_up);
    ev_unref(events.main_loop);

    ev_signal sw[2];
    ev_signal_init(&sw[0], signal_f, SIGINT);
    ev_signal_start(events.main_loop, &sw[0]);

    ev_signal_init(&sw[1], signal_f, SIGTERM);
    ev_signal_start(events.main_loop, &sw[1]);

    // vamos dar tantos unref quantos signal watcher...
    // caso tudo tenha terminado nao queremos trancar o loop
    ev_unref(events.main_loop);
    ev_unref(events.main_loop);

    // setup http server
    events.http_fd = create_listener(HTTP_PORT);
    ev_io_init(&events.http_watcher, accept_http_f, events.http_fd, EV_READ);
    ev_io_start(events.main_loop, &events.http_watcher);

    // setup http server
    events.https_fd = create_listener(HTTPS_PORT);

    events.ssl_ctx = init_ssl(cert_file, key_file);
    if (events.ssl_ctx == NULL)
    {
        fprintf(stderr, "failed to start ssl context\n");
        return 1;
    }

    ev_io_init(&events.https_watcher, accept_secure_f, events.https_fd, EV_READ);
    ev_io_start(events.main_loop, &events.https_watcher);

    // zeroes requisitions
    memset(requests, 0, sizeof(requests));

    // start workers
    threads = (pthread_t *) calloc(n_threads, sizeof(pthread_t));

    for (size_t i=0; i < n_threads; i++)
    {
        pthread_create(&threads[i], NULL,
                       worker_thread, (void *) NULL);
    }

    // timeout para atualizacao dos clientes!
    ev_set_timeout_collect_interval (events.main_loop, 0.1);
    ev_set_io_collect_interval (events.main_loop, 0.01);

    ev_loop(events.main_loop, 0);

    // enqueue workers
    for(i=0; i<n_threads; ++i)
    {
        queue_enqueue(events.job_queue, NULL);
        DEBUG("empty job for %lu", i);
    }

    // wait all threads
    for (i = 0; i < n_threads; ++ i)
    {
        pthread_join(threads[i], NULL);
        DEBUG("thread %lu exited", i);
    }

    // destroy all communication remaining
    if (events.http_fd >= 0)
    {
        shutdown(events.http_fd, SHUT_RDWR);
        close(events.http_fd);
    }

    if (events.https_fd >= 0)
    {
        shutdown(events.https_fd, SHUT_RDWR);
        close(events.https_fd);
    }

    return 0;
}
