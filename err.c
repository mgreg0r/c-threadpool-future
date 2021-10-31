#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "err.h"

void syserr(int err, const char *fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);

    if(!err) err = errno;
    fprintf(stderr," (%d; %s)\n", err, strerror(err));
    exit(1);
}
