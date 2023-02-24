#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    void **buffer;
    int size;
    int start;
    int end;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t empty;
    pthread_cond_t full;
} queue_t;

queue_t *queue_new(int size) {
    queue_t *q = malloc(sizeof(queue_t));
    q->buffer = malloc(size * sizeof(void *));
    q->size = size;
    q->start = 0;
    q->end = 0;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->empty, NULL);
    pthread_cond_init(&q->full, NULL);
    return q;
}

void queue_delete(queue_t **q) {
    pthread_mutex_destroy(&(*q)->lock);
    pthread_cond_destroy(&(*q)->empty);
    pthread_cond_destroy(&(*q)->full);
    free((*q)->buffer);
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {
    pthread_mutex_lock(&q->lock);
    while (q->count == q->size) {
        pthread_cond_wait(&q->full, &q->lock);
    }
    q->buffer[q->end] = elem;
    q->end = (q->end + 1) % q->size;
    q->count++;
    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        pthread_cond_wait(&q->empty, &q->lock);
    }
    *elem = q->buffer[q->start];
    q->start = (q->start + 1) % q->size;
    q->count--;
    pthread_cond_signal(&q->full);
    pthread_mutex_unlock(&q->lock);
    return true;
}
