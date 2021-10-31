#ifndef RUNNABLE_H
#define RUNNABLE_H

typedef struct runnable {
    void (*function)(void *, size_t);
    void *arg;
    size_t argsz;
} runnable_t;

#endif
