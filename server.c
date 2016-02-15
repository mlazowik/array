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

int control_queue, clients_server_queue, server_clients_queue;

int *array;

int create_queue(key_t key) {
    int id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);

    if (id == -1) {
        syserr("Cannot create queue");
    }

    return id;
}

void queue_send(int id, void *msg, size_t msg_size) {
    if (msgsnd(id, msg, msg_size, 0) != 0) {
        syserr("Cannot send to queue");
    }
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

void *worker(void *pid) {
    long client_pid = *((long *) pid);
    free(pid);

    debug("Worker for client with pid %ld created.\n", client_pid);

    Mesg request, response;
    response.mesg_type = client_pid;
    while (true) {
        queue_receive(clients_server_queue, &request, mesg_size(), client_pid);

        if (request.op == QUIT) {
            debug("Worker for client with pid %ld quitting.\n", client_pid);
            return 0;
        }

        if (request.op == READ) {
            response.args[0] = array[request.args[0]];
            queue_send(server_clients_queue, (char *) &response, mesg_size());
        }

        if (request.op == WRITE) {
            array[request.args[0]] = request.args[1];
            queue_send(server_clients_queue, (char *) &response, mesg_size());
        }
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

void exit_server(int sig) {
    remove_queue(control_queue);
    remove_queue(clients_server_queue);
    remove_queue(server_clients_queue);

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

    if ((array = calloc(n, sizeof(int))) == NULL) {
        fatal("Cannot allocate memory.\n");
    }

    control_queue = create_queue(CONTROL_KEY);
    clients_server_queue = create_queue(CLIENTS_SERVER_KEY);
    server_clients_queue = create_queue(SERVER_CLIENTS_KEY);

    Mesg mesg;
    while (true) {
        queue_receive(control_queue, &mesg, mesg_size(), 0);

        debug("Client with pid %ld connected.\n", mesg.mesg_type);

        create_worker(mesg.mesg_type);
    }

    return 0;
}