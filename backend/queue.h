#ifndef _QUEUE_H_
#define _QUEUE_H_

struct Queue;

struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int front(struct Queue* queue);
int rear(struct Queue* queue);
void delete_queue(struct Queue* queue);

#endif 