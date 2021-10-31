#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "future.h"

typedef struct arg {
    int64_t factorial;
    int iteration;
} arg_t;

void* calculate(void* data, size_t data_size, size_t* res_size) {
    arg_t* arg = data;
    arg_t* result = malloc(sizeof(arg_t));;

    if(data == NULL) {
        result->iteration = 0;
        result->factorial = 1;
    } else {
        result->iteration = arg->iteration + 1;
        result->factorial = arg->factorial * result->iteration;
        free(arg);
    }

    *res_size = sizeof(arg_t);
    return result;
}

int main() {
    int n;
    scanf("%d", &n);

    future_t* futures = malloc(sizeof(future_t) * (n + 1));

    callable_t callable = {
            calculate,
            NULL,
            0
    };

    thread_pool_t pool;
    thread_pool_init(&pool, 3);

    async(&pool, &futures[0], callable);

    for(int i = 1; i <= n; i++) {
        map(&pool, &futures[i], &futures[i-1], calculate);
    }

    arg_t* result = await(&futures[n]);

    printf("%ld\n", result->factorial);

    thread_pool_destroy(&pool);
    free(futures);
    return 0;
}
