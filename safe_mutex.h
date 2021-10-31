#ifndef SAFE_MUTEX_H
#define SAFE_MUTEX_H

#include<pthread.h>

void mx_lock(pthread_mutex_t *mx);
void mx_unlock(pthread_mutex_t *mx);
void mx_init(pthread_mutex_t *mx);
void mx_destroy(pthread_mutex_t *mx);


#endif
