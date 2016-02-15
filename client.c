#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int op_time = strtol(argv[1], NULL, 10);

    assert(op_time >= 0);

    debug("Hello client! Got %ld\n", op_time);

    return 0;
}