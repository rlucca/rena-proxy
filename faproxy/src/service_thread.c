#include "globals.h"


#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>


static void dump_block(const char *buffer, size_t len)
{
    for (size_t u = 0; u < len; u++)
    {
        DEBUG("block %p [%ld] = %c || %u",
              buffer, u,
              isprint(buffer[u]) ? buffer[u] : '?',
              buffer[u]);
    }
}

fa_request_t *accept_action_internal(char is_ssl, int client, int side)
{
	SSL *ssl = NULL;
	if (is_ssl)
	{
		DEBUG("Connection SECURE");
		ssl = init_ssl_conn(client);
		if (accept_ssl_conn(ssl) < 0)
		{
			destroy_ssl_conn(ssl);
			ssl = NULL;
			// Deu problema com a comunicacao
			// Nao devemos criar o request e
			// nem o observador de eventos
			return NULL;
		}
		// Em virtude de problemas foi preferido nao deixar o socket nao-bloqueante pro HTTPS
	} else {
		DEBUG("Connection HTTP");
		setnonblock(client);
	}

	fa_request_t *req = fa_request_create(client, ssl, side);

#ifdef DEBUG_MODE_ON
	if (requests[client] != NULL)
		abort();
#endif

	requests[client] = req;
	return req;
}

static void accept_action(const queue_payload_t *work)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int client = -1;

    DEBUG("ACCEPT: fd [%d] action [%d] char [%d]",
          work->fd, work->action, work->is_https);

    while ((client = accept(work->fd, (struct sockaddr*)&addr, &len)) >= 0)
    {
		fa_request_t *ret = accept_action_internal(work->is_https, client, -1);
		DEBUG("New Connection %p: %s:%d", ret, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

		if (ret == NULL)
		{
			continue;
		}

        // after put on requests, start to recv!
        RESTART_EV_IO(&ret->read_watcher);
    }

    if (errno != EAGAIN && errno != EWOULDBLOCK)
    {
        fprintf(stderr, "unknow errno: %d\n", errno);
#ifdef DEBUG_MODE_ON
        abort();
#endif
        return ;
    }

    // all accept's done, restart watcher!
    if (work->is_https)
    {
        RESTART_EV_IO(&events.https_watcher);
    }
    else
    {
        RESTART_EV_IO(&events.http_watcher);
    }
}

static void read_action(const queue_payload_t *work)
{
    fa_request_t *req = requests[work->fd];
    fa_request_t *related = NULL;
    char buf[MAX_STR];
    ssize_t bytes;
    size_t remain;
    if (req == NULL)
    {
        DEBUG("OXI");
        return ;
    }
    DEBUG("READ: HTTP%c fd [%d] action [%d] side [%d] %lu",
          (work->is_https?work->is_https:' '),
          work->fd, work->action, req->side,
          fa_request_size(req));

    remain = fa_request_remain_capacity(req);

    if (remain == 0)
    {
        DEBUG("no capacity to read action on http%c socket [%d]",
              (work->is_https?work->is_https:' '),
              work->fd);
        return ;
    }

    fa_request_curr_state_set(
            req,
            fa_request_curr_state(req) | STATE_READ);

    errno = 0;
    if (req->ctx == NULL)
    {
        bytes = recv(req->fd, buf, remain, MSG_DONTWAIT);
        for (int K=0;
             K < 3 && bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK);
             K++)
        {
            errno = 0;
            bytes = recv(req->fd, buf, remain, MSG_DONTWAIT);
        }
    } else {
        bytes = SSL_read(req->ctx, buf, remain);
        // em teoria nao daria o errno aqui
    }

    DEBUG("REQ_CTX read ssl [%p] bytes [%d] errno [%d:%m]", req->ctx, bytes, errno);

    if (bytes > 0)
    {
        req->n_bytes += bytes;

        (void) dump_block; //(buf, bytes);
        if (fa_request_payload_transform(req, buf, bytes) < 0)
        {
            // quando deu erro vamos encerrar!
            bytes = 0;
        }
    } else {
        DEBUG("remain capacity %lu on bytes 0", fa_request_remain_capacity(req));
    }

    if(bytes == 0)
    {
        if (fa_request_push_holding(req))
        {
            return ;
        }

        fa_request_curr_state_set(
                req,
                fa_request_curr_state(req) & ~STATE_READ);

        if (fa_request_delete(req) == 0) // sucess!
            DEBUG("bytes 0: %p (%d) %d deleted", requests[work->fd], STATE_READ, work->fd); // invalid req->fd and req->ctx and req
        return ;
    }

    if (req != NULL && req->related_fd >= 0)
        related = requests[req->related_fd];

    if (related && fa_request_size(related) > 0)
    {
        RESTART_EV_IO(&related->write_watcher);
        return ;
    } else {
        if (fa_request_remain_capacity(req) > 0
            && (related == NULL
            || fa_request_remain_capacity(related) > 0))
        {
            RESTART_EV_IO(&req->read_watcher);
        }
    }
}

static void write_action(const queue_payload_t *work)
{
    fa_request_t *req = requests[work->fd];
    fa_request_t *related = NULL;
    char buf[MAX_STR];
    int bytes;
    size_t sz;
    if (req == NULL)
    {
        DEBUG("IXI");
        return ;
    }
    DEBUG("WRITE: HTTP%1c fd [%d] action [%d] side [%d] %lu",
          (work->is_https?work->is_https:' '),
          work->fd, work->action, req->side,
          fa_request_size(req));

    if (req != NULL && req->related_fd >= 0)
        related = requests[req->related_fd];

    sz = fa_request_size(req);
    if (sz == 0)
    {
        DEBUG("nothing to write on write action on http%c socket [%d]",
              (work->is_https?work->is_https:' '),
              work->fd);

        fa_request_curr_state_set(
                req,
                fa_request_curr_state(req) & ~STATE_WRITE);

        // avisamos a outra ponta que precisamos de dados...
        if (related)
            RESTART_EV_IO(&related->read_watcher);
        // necessidade de encerrar?
        RESTART_EV_IO(&req->read_watcher);
        return ;
    }

    fa_request_curr_state_set(
            req,
            fa_request_curr_state(req) | STATE_WRITE);

    sz = fa_request_payload_copy(req, buf, sz);

    if (sz <= 0)
    {
        DEBUG("nothing to write on write action on http%c socket [%d]",
              (work->is_https?work->is_https:' '),
              work->fd);

        fa_request_curr_state_set(
                req,
                fa_request_curr_state(req) & ~STATE_WRITE);

        // avisamos a outra ponta que precisamos de dados...
        if (related)
            RESTART_EV_IO(&related->read_watcher);
        return ;
    }

    if (req->ctx == NULL)
    {
        bytes = write(req->fd, buf, sz);
        if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            bytes = write(req->fd, buf, sz);
    } else {
        bytes = SSL_write(req->ctx, buf, sz);
        DEBUG("ssl_write [%d] %d %m", bytes, errno);
    }

    if (bytes > 0)
    {
        fa_request_payload_consume(req, bytes);
    }
    DEBUG("WRITE: bytes %d > 0 (%ld) -=> %ld", bytes, sz, fa_request_size(req));

    if (bytes == 0)
    {
        fa_request_curr_state_set(
                req,
                fa_request_curr_state(req) & ~STATE_WRITE);
    }

    if (fa_request_size(req) > 0)
    {
        RESTART_EV_IO(&req->write_watcher);
    } else {
        if (related)
        {
            // mande mais dados e consuma!
            RESTART_EV_IO(&related->read_watcher);
            RESTART_EV_IO(&related->write_watcher);
        }
    }
}


void *worker_thread(void *nulled)
{
    queue_payload_t *work = NULL;

    DEBUG("waiting job...");
    while ((work = queue_dequeue(events.job_queue)) != NULL)
    {
        switch (work->action)
        {
            case ACTION_READ:
                read_action(work);
                break;

            case ACTION_WRITE:
                write_action(work);
                break;

            case ACTION_ACCEPT:
                accept_action(work);
                break;

            case ACTION_NONE:
            default:
                DEBUG("UNKNOWN ACTION: fd [%d] action [%d] char [%d]",
                      work->fd, work->action, work->is_https);
                break;
        }

        DEBUG("waiting job...");
    }

    DEBUG("thread exit");
    return NULL;
}
