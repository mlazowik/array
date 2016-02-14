#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int op_time = strtol(argv[1], NULL, 10);

    assert(op_time >= 0);

    printf("Hello client! Got %ld\n", op_time);

    return 0;
}