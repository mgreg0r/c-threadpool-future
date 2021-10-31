#ifndef FUTURE_H
#define FUTURE_H

#include <pthread.h>
#include <semaphore.h>
#include "threadpool.h"

typedef struct callable {
  void *(*function)(void*, size_t, size_t*);
  void* arg;
  size_t argsz;
} callable_t;

typedef struct future {
    void* result;
    size_t result_size;
    int8_t computed;
    int8_t waiting;
    pthread_mutex_t lock;
    sem_t wait_for_result;
} future_t;

int async(thread_pool_t* pool, future_t* future, callable_t callable);

int map(thread_pool_t* pool, future_t* future, future_t* from,
        void *(*function)(void*, size_t, size_t*));

void* await(future_t* future);

#endif
