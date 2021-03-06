#ifndef QUEUE_H
#define QUEUE_H

typedef struct qnode {
    void*         data;
    struct qnode* prev;
    struct qnode* next;
} qnode;

typedef struct queue {
    qnode* head;
    qnode* tail;
    pthread_mutex_t mutex;
	pthread_cond_t not_empty;
	int closed;
} queue;

queue* make_queue();
void free_queue(queue* qq);

void queue_put(queue* qq, void* msg);
void* queue_get(queue* qq);

#endif
