#include <stdlib.h>
#include "job_queue.h"

node_t* create_node(runnable_t* job) {
    node_t* node = malloc(sizeof(node_t));
    node->job = job;
    node->next = NULL;
    return node;
}

job_queue_t* create_queue() {
    job_queue_t* queue = malloc(sizeof(job_queue_t));
    queue->begin = queue->end = create_node(NULL);
    queue->size = 0;
    return queue;
}

void destroy_queue(job_queue_t* q) {
    node_t* node = q->begin;
    while(node != NULL) {
        node_t* next = node->next;
        if(node->job != NULL)
            free(node->job);
        free(node);
        node = next;
    }

    free(q);
}

void insert(job_queue_t* q, runnable_t* job) {
    node_t* node = create_node(job);
    q->end->next = node;
    q->end = node;
    q->size++;
}

runnable_t* fetch(job_queue_t* q) {
    runnable_t* result = q->begin->next->job;
    node_t* new_next = q->begin->next->next;
    if(q->begin->next == q->end) {
        q->end = q->begin;
    }
    free(q->begin->next);
    q->begin->next = new_next;
    q->size--;
    return result;
}
