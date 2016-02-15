#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "err.h"

#ifndef NDEBUG
const bool DEBUG = true;
#else
const bool DEBUG = false;
#endif

extern int sys_nerr;

void debug(const char *fmt, ...) {
    if (DEBUG) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

void syserr_errno(int err, const char *fmt, ...)
{
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr," (%d; %s)\n", err, strerror(err));
    exit(1);
}

void syserr(const char *fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr," (%d; %s)\n", errno, strerror(errno));
    exit(1);
}

void fatal(const char *fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);

    fprintf(stderr,"\n");
    exit(1);
}
