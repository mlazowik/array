#ifndef ARRAY_ERR_H
#define ARRAY_ERR_H

#include <stdbool.h>

extern const bool DEBUG;

extern void debug(const char *fmt, ...);

/* print system call error message and terminate */
extern void syserr_errno(int err, const char *fmt, ...);

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie */
extern void syserr(const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

#endif //ARRAY_ERR_H
