#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"
#include "mesg.h"

int msg_qid;

void exit_server(int sig) {
    if (msgctl(msg_qid, IPC_RMID, 0) == -1) {
        syserr("msgctl RMID");
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int n = strtol(argv[1], NULL, 10);

    assert(n >= 2);

    debug("Hello server! Got %ld.\n", n);

    if (signal(SIGINT,  exit_server) == SIG_ERR) {
        syserr("signal");
    }

    if ((msg_qid = msgget(MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
        syserr("msgget");
    }

    Mesg mesg;
    while (true) {
        if (msgrcv(msg_qid, &mesg, mesg_size(), 0, 0) <= 0) {
            syserr("msgrcv");
        }

        debug("Client with pid %d connected.\n", mesg.mesg_type);
    }

    return 0;
}