#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int n = strtol(argv[1], NULL, 10);

    assert(n >= 2);

    debug("Hello server! Got %ld\n", n);

    return 0;
}