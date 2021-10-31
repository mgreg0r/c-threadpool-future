#include <stdlib.h>
#include <stdio.h>
#include "future.h"
#include "safe_mutex.h"
#include "err.h"

typedef struct job_arg {
    callable_t callable;
    future_t* future;
} job_arg_t;

void worker(void* data, size_t data_size) {
    job_arg_t* arg = data;

    arg->future->result = arg->callable.function(arg->callable.arg, arg->callable.argsz, &arg->future->result_size);

    mx_lock(&arg->future->lock);
    arg->future->computed = 1;
    if(arg->future->waiting) {
        mx_unlock(&arg->future->lock);
        int err = sem_post(&arg->future->wait_for_result);
        if(err) syserr(0, "sem post failed");
    } else {
        mx_unlock(&arg->future->lock);
    }
}

typedef struct map_arg {
    future_t* dep;
    void *(*function)(void*, size_t, size_t*);
} map_arg_t;

void* map_worker(void* data, size_t data_size, size_t* res_size) {
    map_arg_t* arg = data;
    void* from_result = await(arg->dep);
    size_t from_size = arg->dep->result_size;
    void* result = arg->function(from_result, from_size, res_size);
    free(arg);
    return result;
}

job_arg_t* create_job_arg(future_t* future, callable_t callable) {
    job_arg_t* arg = malloc(sizeof(job_arg_t));
    arg->callable = callable;
    arg->future = future;
    arg->future->computed = 0;
    arg->future->waiting = 0;
    mx_init(&arg->future->lock);
    int err = sem_init(&future->wait_for_result, 0, 0);
    if(err) syserr(0, "semaphore init failed");
    return arg;
}

int async(thread_pool_t* pool, future_t* future, callable_t callable) {
    job_arg_t* arg = create_job_arg(future, callable);
    runnable_t job = {
            worker,
            arg,
            sizeof(job_arg_t)
    };

    defer(pool, job);
    return 0;
}

int map(thread_pool_t* pool, future_t* future, future_t* from,
        void *(*function)(void*, size_t, size_t*)) {
    map_arg_t* arg = malloc(sizeof(map_arg_t));
    arg->dep = from;
    arg->function = function;

    callable_t callable = {
            map_worker,
            arg,
            sizeof(map_arg_t)
    };

    return async(pool, future, callable);
}

void *await(future_t* future) {
    mx_lock(&future->lock);
    if(!future->computed) {
        future->waiting = 1;
        mx_unlock(&future->lock);
        int err = sem_wait(&future->wait_for_result);
        if(err) syserr(0, "sem wait failed");
    } else {
        mx_unlock(&future->lock);
    }

    mx_destroy(&future->lock);
    int err = sem_destroy(&future->wait_for_result);
    if(err) syserr(0, "sem destroy failed");

    return future->result;
}
