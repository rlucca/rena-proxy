#include "globals.h"
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>


#include <sys/socket.h>
#include <arpa/inet.h>



pthread_t *threads = NULL;
size_t n_threads = 4;
fa_request_t *requests[MAX_FDS];
fa_control_t events;


// it's called in main and after accept
int setnonblock(int fd)
{
    int flags;
    int up = 1;
    //int few = 64;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &up, sizeof(up));
    //setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &few, sizeof(few));
    //setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &few, sizeof(few));

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

int prepare_socket(int port)
{
    int sd;
    struct sockaddr_in addr;
    int activate = 1;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *events.bind_addr;

    DEBUG("ADDR: %s / %d",
          inet_ntoa(addr.sin_addr),
          htons(addr.sin_port));

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &activate, sizeof(int)) < 0)
    {
        DEBUG("setsockopt(SO_REUSEADDR) failed");
    }

    errno = 0;
    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        perror("can't bind address and port");
        return -1;
    }

    return sd;
}

int connect_to_server(const char *host, int port)
{
    char hostname[MAX_STR];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    int sfd = -1;
    char *change_port = strchr(host, ':');
    int hostname_len =
            (change_port) ? change_port - host : strnlen(host, MAX_STR);

    DEBUG("TODO: precisa ser enfileirado para ganhar velocidade!");
    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    strncpy(hostname, host, hostname_len);
    hostname[MAX_STR - 1] = 0;

    if (change_port)
        port = atoi(change_port + 1);

    s = getaddrinfo(hostname, NULL, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s |%s|\n", gai_strerror(s), host);
        return -1;
    }

    // Tenta conectar, se todos falharem devolver erro
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        sfd = prepare_socket(0);
        if (sfd < 0)
            continue;

        ((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(port);

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) >= 0)
        {
            break;                  /* Success */
        }

        close(sfd);
        sfd = -1;
    }

    freeaddrinfo(result);
    setnonblock(sfd);

    return sfd;
}
