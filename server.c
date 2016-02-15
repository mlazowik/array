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
#include "array.h"

int control_queue, clients_server_queue, server_clients_queue;

void remove_queue(int id) {
    if (msgctl(id, IPC_RMID, 0) == -1) {
        debug("Cannot remove queue");
    }
}

void clean() {
    remove_queue(control_queue);
    remove_queue(clients_server_queue);
    remove_queue(server_clients_queue);
}

int create_queue(key_t key) {
    int id = msgget(key, 0666 | IPC_CREAT | IPC_EXCL);

    if (id == -1) {
        clean();
        syserr("Cannot create queue");
    }

    return id;
}

void queue_send(int id, void *msg) {
    if (msgsnd(id, msg, mesg_size(), 0) != 0) {
        clean();
        syserr("Cannot send to queue");
    }
}

void queue_receive(int id, void *msg, long type) {
    if (msgrcv(id, msg, mesg_size(), type, 0) <= 0) {
        clean();
        syserr("Cannot receive from queue");
    }
}

int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void *worker(void *pid) {
    long client_pid = *((long *) pid);
    free(pid);

    debug("Worker for client with pid %ld created.\n", client_pid);

    Mesg request, response;
    response.mesg_type = client_pid;
    while (true) {
        queue_receive(clients_server_queue, &request, client_pid);

        if (request.op == QUIT) {
            debug("Worker for client with pid %ld quitting.\n", client_pid);
            return 0;
        }

        if (request.op == READ) {
            size_t index = request.args[0];

            lock_read(index);
            response.args[0] = get_value(index);
            unlock_read(index);

            queue_send(server_clients_queue, (char *) &response);
        }

        if (request.op == WRITE) {
            size_t index = request.args[0];

            lock_write(index);
            set_value(request.args[0], request.args[1]);
            unlock_write(index);

            queue_send(server_clients_queue, (char *) &response);
        }

        if (request.op == SUM_GET) {
            int sorted[request.args_count];

            for (size_t i = 0; i < request.args_count; i++) {
                sorted[i] = request.args[i];
            }

            qsort(sorted, request.args_count, sizeof(int), compare);

            for (size_t i = 0; i < request.args_count; i++) {
                if (i == 0 || sorted[i-1] != sorted[i]) {
                    if (sorted[i] == request.args[0]) {
                        lock_write(sorted[i]);
                    } else {
                        lock_read(sorted[i]);
                    }
                }
            }

            for (size_t i = 1; i < request.args_count; i++) {
                response.args[i] = get_value(request.args[i]);
            }

            queue_send(server_clients_queue, (char *) &response);
        }

        if (request.op == SUM_SET) {
            set_value(request.args[0], request.args[request.args_count - 1]);

            int sorted[request.args_count - 1];

            for (size_t i = 0; i < request.args_count; i++) {
                sorted[i] = request.args[i];
            }

            qsort(sorted, request.args_count - 1, sizeof(int), compare);

            for (size_t i = 0; i < request.args_count - 1; i++) {
                if (i == 0 || sorted[i-1] != sorted[i]) {
                    if (sorted[i] == request.args[0]) {
                        unlock_write(sorted[i]);
                    } else {
                        unlock_read(sorted[i]);
                    }
                }
            }

            queue_send(server_clients_queue, (char *) &response);
        }

        if (request.op == SWAP_GET) {
            if (request.args[0] == request.args[1]) {
                lock_write(request.args[0]);
            } else if (request.args[0] < request.args[1]) {
                lock_write(request.args[0]);
                lock_write(request.args[1]);
            } else {
                lock_write(request.args[1]);
                lock_write(request.args[0]);
            }

            queue_send(server_clients_queue, (char *) &response);
        }

        if (request.op == SWAP_SET) {
            int tmp = get_value(request.args[0]);
            set_value(request.args[0], get_value(request.args[1]));
            set_value(request.args[1], tmp);

            if (request.args[0] == request.args[1]) {
                unlock_write(request.args[0]);
            } else if (request.args[0] < request.args[1]) {
                unlock_write(request.args[0]);
                unlock_write(request.args[1]);
            } else {
                unlock_write(request.args[1]);
                unlock_write(request.args[0]);
            }

            queue_send(server_clients_queue, (char *) &response);
        }
    }

    return 0;
}

void create_worker(pid_t pid) {
    pthread_t thread;
    int err;

    long *pid_ptr;
    if ((pid_ptr = malloc(sizeof(*pid_ptr))) == NULL) {
        clean();
        fatal("Cannot allocate memory.\n");
    }

    *pid_ptr = pid;

    if ((err = pthread_create(&thread, NULL, worker, pid_ptr)) != 0) {
        clean();
        syserr_errno(err, "create");
    }
}

void exit_server(int sig) {
    clean();
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

    array_init(n);

    control_queue = create_queue(CONTROL_KEY);
    clients_server_queue = create_queue(CLIENTS_SERVER_KEY);
    server_clients_queue = create_queue(SERVER_CLIENTS_KEY);

    Mesg mesg;
    while (true) {
        queue_receive(control_queue, &mesg, 0);

        debug("Client with pid %ld connected.\n", mesg.mesg_type);

        create_worker(mesg.mesg_type);
    }

    return 0;
}