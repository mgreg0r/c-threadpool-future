#include <pthread.h>
#include "err.h"

void mx_lock(pthread_mutex_t *mx) {
    int err = pthread_mutex_lock(mx);
    if(err) syserr(err, "mutex lock failed");
}

void mx_unlock(pthread_mutex_t *mx) {
    int err = pthread_mutex_unlock(mx);
    if(err) syserr(err, "mutex unlock failed");
}

void mx_init(pthread_mutex_t *mx) {
    int err = pthread_mutex_init(mx, 0);
    if(err) syserr(err, "mutex init failed");
}

void mx_destroy(pthread_mutex_t *mx) {
    int err = pthread_mutex_destroy(mx);
    if(err) syserr(err, "mutex destroy failed");
}
