#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
#include "job_queue.h"
#include "runnable.h"

typedef struct thread_pool {
  sem_t job_semaphore;
  pthread_mutex_t lock;
  job_queue_t* jobs;
  size_t size;
  size_t idle_threads;
  int8_t destroyed;
  pthread_t** threads;
  pthread_attr_t thread_attr;
} thread_pool_t;

int thread_pool_init(thread_pool_t* pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t* pool);

int defer(thread_pool_t* pool, runnable_t runnable);

#endif
