#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "safe_mutex.h"
#include "threadpool.h"
#include "job_queue.h"
#include "err.h"

int INTERRUPTED = 0;

void catch_sigint() {
    INTERRUPTED = 1;
}

void *worker_function(void* data) {
    thread_pool_t* pool = data;
    while(1) {
        mx_lock(&pool->lock);
        if(!pool->destroyed) {
            pool->idle_threads++;
            mx_unlock(&pool->lock);
            int err = sem_wait(&pool->job_semaphore);
            if(err) syserr(0, "semaphore wait failed");
            mx_lock(&pool->lock);
            pool->idle_threads--;
        }

        if(pool->jobs->size == 0) {
            mx_unlock(&pool->lock);
            return NULL;
        }

        runnable_t* job = fetch(pool->jobs);

        mx_unlock(&pool->lock);
        job->function(job->arg, job->argsz);
        free(job);
    }
}

int thread_pool_init(thread_pool_t* pool, size_t num_threads) {
    int err = sem_init(&pool->job_semaphore, 0, 0);
    if(err) syserr(0, "semaphore init failed");

    mx_init(&pool->lock);

    pool->jobs = create_queue();
    pool->idle_threads = 0;
    pool->size = num_threads;
    pool->destroyed = 0;

    err = pthread_attr_init(&pool->thread_attr);
    if(err) syserr(0, "pthread attr init failed");

    err = pthread_attr_setdetachstate(&pool->thread_attr, PTHREAD_CREATE_JOINABLE);
    if(err) syserr(0, "pthread attr setting failed");

    pool->threads = malloc(sizeof(pthread_t*) * num_threads);

    for(size_t i = 0; i < num_threads; i++) {
        pool->threads[i] = malloc(sizeof(pthread_t));
        err = pthread_create(pool->threads[i], &pool->thread_attr, worker_function, pool);
        if(err) syserr(0, "thread create failed");
    }

    struct sigaction action;
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);
    action.sa_handler = catch_sigint;
    action.sa_mask = block_mask;
    action.sa_flags = 0;
    err = sigaction(SIGINT, &action, 0);
    if(err) syserr(err, "sigaction failed");

    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {
    int err;

    mx_lock(&pool->lock);
    pool->destroyed = 1;

    for(size_t i = 0; i < pool->idle_threads; i++) {
        err = sem_post(&pool->job_semaphore);
        if(err) syserr(0, "semaphore post failed");
    }

    mx_unlock(&pool->lock);

    for(size_t i = 0; i < pool->size; i++) {
        err = pthread_join(*pool->threads[i], NULL);
        if(err) syserr(0, "thread join failed");
    }

    err = sem_destroy(&pool->job_semaphore);
    if(err) syserr(0, "semaphore destroy failed");

    destroy_queue(pool->jobs);
    mx_destroy(&pool->lock);

    err = pthread_attr_destroy(&pool->thread_attr);
    if(err) syserr(0, "thread attr destroy failed");

    free(pool->threads);
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    int err = pthread_mutex_lock(&pool->lock);
    if(err) return -1;

    if(pool->destroyed || INTERRUPTED) {
        mx_unlock(&pool->lock);
        return -1;
    }

    runnable_t* job = malloc(sizeof(runnable_t));
    *job = runnable;
    insert(pool->jobs, job);
    err = sem_post(&pool->job_semaphore);
    if(err) syserr(0, "semaphore post failed");

    mx_unlock(&pool->lock);

    return 0;
}
