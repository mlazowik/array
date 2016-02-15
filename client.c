#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"
#include "mesg.h"

int msg_qid;

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int op_time = strtol(argv[1], NULL, 10);

    assert(op_time >= 0);

    debug("Hello client! Got %ld.\n", op_time);

    if ((msg_qid = msgget(MKEY, 0)) == -1) {
        syserr("msgget");
    }

    debug("My pid is %d.\n", getpid());

    Mesg mesg;
    mesg.mesg_type = getpid();

    if (msgsnd(msg_qid, (char *) &mesg, mesg_size(), 0) != 0) {
        syserr("msgsnd");
    }

    return 0;
}