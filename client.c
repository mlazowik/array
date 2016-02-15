#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"
#include "mesg.h"

int control_queue, clients_queue;

int get_queue(key_t key) {
    int id;

    id = msgget(key, 0);

    if (id == -1) {
        syserr("Cannot get queue");
    }

    return id;
}

void queue_send(int id, void *msg, size_t msg_size) {
    if (msgsnd(id, msg, msg_size, 0) != 0) {
        syserr("Cannot send to queue");
    }
}

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int op_time = strtol(argv[1], NULL, 10);

    assert(op_time >= 0);

    debug("Hello client! Got %ld.\n", op_time);

    control_queue = get_queue(CONTROL_KEY);
    clients_queue = get_queue(CLIENTS_KEY);

    debug("My pid is %ld.\n", getpid());

    Mesg mesg;
    mesg.mesg_type = getpid();

    queue_send(control_queue, (char *) &mesg, mesg_size());

    return 0;
}