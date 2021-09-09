/*
c-pthread-queue - c implementation of a bounded buffer queue using posix threads
Copyright (C) 2008  Matthew Dickinson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <pthread.h>

#ifndef _QUEUE_H
#define _QUEUE_H

#define QUEUE_INITIALIZER(buffer) { buffer, sizeof(buffer) / sizeof(buffer[0]), 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }

typedef enum {
    ACTION_NONE = 0,
    ACTION_ACCEPT,
    ACTION_READ,
    ACTION_WRITE
} action_e;

typedef struct queue_payload {
    int fd;
    action_e action;
    char is_https;
} queue_payload_t;

typedef struct queue
{
	queue_payload_t **buffer;
	const int capacity;
	int size;
	int in;
	int out;
	pthread_mutex_t mutex;
	pthread_cond_t cond_full;
	pthread_cond_t cond_empty;
} queue_t;

queue_payload_t *queue_payload(action_e action, int fd, char is_https);
void queue_enqueue(queue_t *queue, void *value);
void *queue_dequeue(queue_t *queue);
int queue_size(queue_t *queue);

#endif
