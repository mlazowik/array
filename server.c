#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"
#include "mesg.h"

int control_queue, clients_queue;

int create_queue(key_t key) {
    int id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);

    if (id == -1) {
        syserr("Cannot create queue");
    }

    return id;
}

void queue_receive(int id, void *msg, size_t msg_size, long type) {
    if (msgrcv(id, msg, msg_size, type, 0) <= 0) {
        syserr("Cannot receive from queue");
    }
}

void remove_queue(int id) {
    if (msgctl(id, IPC_RMID, 0) == -1) {
        syserr("Cannot remove queue");
    }
}

void exit_server(int sig) {
    remove_queue(control_queue);
    remove_queue(clients_queue);

    exit(0);
}

void *worker(void *pid) {
    long client_pid = *((long *) pid);
    free(pid);

    debug("Worker for client with pid %ld created.\n", client_pid);

    Mesg mesg;
    while (true) {
        queue_receive(clients_queue, &mesg, mesg_size(), client_pid);
    }

    return 0;
}

void create_worker(pid_t pid) {
    pthread_t thread;
    int err;

    long *pid_ptr;
    if ((pid_ptr = malloc(sizeof(*pid_ptr))) == NULL) {
        fatal("Cannot allocate memory.\n");
    }

    *pid_ptr = pid;

    if ((err = pthread_create(&thread, NULL, worker, pid_ptr)) != 0) {
        syserr_errno(err, "create");
    }
}

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int n = strtol(argv[1], NULL, 10);

    assert(n >= 2);

    debug("Hello server! Got %ld.\n", n);

    if (signal(SIGINT,  exit_server) == SIG_ERR) {
        syserr("signal");
    }

    control_queue = create_queue(CONTROL_KEY);
    clients_queue = create_queue(CLIENTS_KEY);

    Mesg mesg;
    while (true) {
        queue_receive(control_queue, &mesg, mesg_size(), 0);

        debug("Client with pid %ld connected.\n", mesg.mesg_type);

        create_worker(mesg.mesg_type);
    }

    return 0;
}