#Assignment 3 directory

This directory contains source code and other files for Assignment 3.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

I made a main to test my code:

//
// Created by Gea Loro on 2/23/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

#define THREAD_COUNT 10
#define QUEUE_SIZE 5

void *producer(void *arg);
void *consumer(void *arg);

int main(void)
{
    queue_t *q = queue_new(QUEUE_SIZE);
    if (!q) {
        fprintf(stderr, "Failed to create queue\n");
        return EXIT_FAILURE;
    }

    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT / 2; i++) {
        pthread_create(&threads[i], NULL, producer, q);
    }
    for (int i = THREAD_COUNT / 2; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, consumer, q);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    queue_delete(&q);

    return EXIT_SUCCESS;
}

void *producer(void *arg)
{
    queue_t *q = (queue_t *)arg;

    for (int i = 0; i < QUEUE_SIZE; i++) {
        int *elem = malloc(sizeof(int));
        *elem = i;
        if (queue_push(q, elem)) {
            printf("Producer %ld pushed %d\n", pthread_self(), *elem);
        } else {
            printf("Producer %ld failed to push %d\n", pthread_self(), *elem);
        }
    }

    return NULL;
}

void *consumer(void *arg)
{
    queue_t *q = (queue_t *)arg;

    for (int i = 0; i < QUEUE_SIZE; i++) {
        int *elem = NULL;
        if (queue_pop(q, (void **)&elem)) {
            printf("Consumer %ld popped %d\n", pthread_self(), *elem);
            free(elem);
        } else {
            printf("Consumer %ld failed to pop\n", pthread_self());
        }
    }

    return NULL;
}

My code returned:

Producer 6128660480 pushed 0
Producer 6128660480 pushed 1
Producer 6129233920 pushed 0
Consumer 6131527680 popped 0
Consumer 6131527680 popped 0
Producer 6128660480 pushed 2
Producer 6128660480 pushed 3
Producer 6128660480 pushed 4
Producer 6129807360 pushed 0
Consumer 6131527680 popped 0
Consumer 6132101120 popped 0
Consumer 6133248000 popped 3
Consumer 6132101120 popped 4
Producer 6129807360 pushed 1
Producer 6129233920 pushed 1
Producer 6129807360 pushed 2
Producer 6129233920 pushed 2
Producer 6129807360 pushed 3
Producer 6129233920 pushed 3
Producer 6129807360 pushed 4
Producer 6130380800 pushed 0
Consumer 6133248000 popped 1
Consumer 6133821440 popped 0
Consumer 6132101120 popped 1
Consumer 6132101120 popped 3
Consumer 6132101120 popped 3
Consumer 6132674560 popped 2
Consumer 6132674560 popped 4
Consumer 6132674560 popped 4
Consumer 6132674560 popped 1
Consumer 6133821440 popped 2
Producer 6130380800 pushed 1
Producer 6130954240 pushed 0
Producer 6130954240 pushed 1
Producer 6130954240 pushed 2
Producer 6130954240 pushed 3
Consumer 6131527680 popped 1
Consumer 6133248000 popped 2
Consumer 6131527680 popped 1
Producer 6129233920 pushed 4
Producer 6130954240 pushed 4
Consumer 6133248000 popped 2
Consumer 6133248000 popped 4
Producer 6130380800 pushed 2
Consumer 6132674560 popped 3
Producer 6130380800 pushed 3
Producer 6130380800 pushed 4
Consumer 6133821440 popped 2
Consumer 6133821440 popped 3
Consumer 6133821440 popped 4

Process finished with exit code 0
