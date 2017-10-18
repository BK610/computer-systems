#include <stdlib.h>
#include <assert.h>

#include "queue.h"

queue* 
make_queue()
{
    queue* qq = malloc(sizeof(queue));
    qq->head = 0;
    qq->tail = 0;
    pthread_mutex_init(&qq->mutex, 0);
    pthread_cond_init(&qq->not_empty, 0);
    qq->closed = 0;
    return qq;
}

void 
free_queue(queue* qq)
{
    assert(qq->head == 0 && qq->tail == 0);
    free(qq);
}

void 
queue_put(queue* qq, void* msg)
{
    qnode* node = malloc(sizeof(qnode));
    node->data = msg;
    node->prev = 0;
    node->next = 0;
    
    pthread_mutex_lock(&qq->mutex); // lock to add to queue
    node->next = qq->head;
    qq->head = node;

    if (node->next) {
        node->next->prev = node;
    }
    else {
        qq->tail = node;
    }
    
    pthread_cond_signal(&qq->not_empty); // good to go!
    pthread_mutex_unlock(&qq->mutex); // unlock queue
}

void* 
queue_get(queue* qq)
{
    pthread_mutex_lock(&qq->mutex);
    
    if (!qq->tail) {
        // FIXME: We should block here.
        // printf("waiting for not_empty\n");
        pthread_cond_wait(&qq->not_empty, &qq->mutex);
        if(qq->closed) {
            pthread_mutex_unlock(&qq->mutex);
            return 0;
        }
    }

    qnode* node = qq->tail;

    if (node->prev) {
        qq->tail = node->prev;
        node->prev->next = 0;
    }
    else {
        qq->head = 0;
        qq->tail = 0;
    }

    void* msg = node->data;
    free(node);
    pthread_mutex_unlock(&qq->mutex);
    return msg;
}