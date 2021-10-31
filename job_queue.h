#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include "runnable.h"

typedef struct node {
    runnable_t* job;
    struct node* next;
} node_t;

typedef struct job_queue {
    node_t* begin;
    node_t* end;
    size_t size;
} job_queue_t;

job_queue_t* create_queue();
void destroy_queue(job_queue_t* queue);
void insert(job_queue_t* q, runnable_t* job);
runnable_t* fetch(job_queue_t* q);

#endif
