#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "threadpool.h"
#include "safe_mutex.h"

typedef struct result {
    int* sums;
    pthread_mutex_t lock;
} result_t;

typedef struct job_arg {
    int value;
    int delay;
    result_t* result;
    int row_index;
} job_arg_t;

void worker(void *data, size_t size) {
    job_arg_t* arg = data;
    usleep(arg->delay * 1000);
    mx_lock(&arg->result->lock);

    arg->result->sums[arg->row_index] += arg->value;
    mx_unlock(&arg->result->lock);

    free(arg);
}

runnable_t create_job(int val, int delay, result_t* result, int index) {
    job_arg_t* arg = malloc(sizeof(job_arg_t));
    arg->value = val;
    arg->delay = delay;
    arg->result = result;
    arg->row_index = index;

    runnable_t job = {
            worker,
            arg,
            sizeof(job_arg_t)
    };

    return job;
}

int main() {
    thread_pool_t pool;
    thread_pool_init(&pool, 4);
    int k, n, val, delay;

    scanf("%d%d", &k, &n);

    result_t result;
    result.sums = malloc(k * sizeof(int));
    mx_init(&result.lock);

    for(int i = 0; i < k; i++) {
        for(int j = 0; j < n; j++) {
            scanf("%d%d", &val, &delay);
            defer(&pool, create_job(val, delay, &result, i));
        }
    }

    thread_pool_destroy(&pool);

    for(int i = 0; i < k; i++) {
        printf("%d\n", result.sums[i]);
    }

    mx_destroy(&result.lock);
    free(result.sums);
    return 0;
}
