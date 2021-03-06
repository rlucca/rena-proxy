#include "globals.h"

queue_payload_t *queue_payload(action_e action, int fd, char is_https)
{
    queue_payload_t *payload = malloc(sizeof(queue_payload_t));
    payload->action = action;
    payload->fd = fd;
    payload->is_https = is_https;
    return payload;
}

void queue_enqueue(queue_t *queue, void *value)
{
	pthread_mutex_lock(&(queue->mutex));
	while (queue->size == queue->capacity)
		pthread_cond_wait(&(queue->cond_full), &(queue->mutex));
	//printf("enqueue %d\n", *(int *)value);
	queue->buffer[queue->in] = value;
	++ queue->size;
	++ queue->in;
	queue->in %= queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));
	pthread_cond_broadcast(&(queue->cond_empty));
}

void *queue_dequeue(queue_t *queue)
{
	pthread_mutex_lock(&(queue->mutex));
	while (queue->size == 0)
		pthread_cond_wait(&(queue->cond_empty), &(queue->mutex));
	void *value = queue->buffer[queue->out];
	//printf("dequeue %d\n", *(int *)value);
	-- queue->size;
	++ queue->out;
	queue->out %= queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));
	pthread_cond_broadcast(&(queue->cond_full));
	return value;
}

int queue_size(queue_t *queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int size = queue->size;
	pthread_mutex_unlock(&(queue->mutex));
	return size;
}
