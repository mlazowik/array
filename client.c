#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"
#include "mesg.h"

int control_queue, clients_server_queue, server_clients_queue;

long pid;

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

void queue_receive(int id, void *msg, size_t msg_size, long type) {
    if (msgrcv(id, msg, msg_size, type, 0) <= 0) {
        syserr("Cannot receive from queue");
    }
}

void read_args(const char *p, Mesg *msg) {
    int n, count = 0;

    while (sscanf(p, "%d%n", &msg->args[count], &n) == 1) {
        p += n;
        count++;
    }

    msg->args_count = count;
}

void op_read(const char *p) {
    Mesg request, response;

    request.mesg_type = pid;
    request.op = READ;

    read_args(p, &request);

    queue_send(clients_server_queue, (char *) &request, mesg_size());

    queue_receive(server_clients_queue, (char *) &response, mesg_size(), pid);

    printf("r %d %d\n", request.args[0], response.args[0]);
}

void op_write(const char *p) {
    Mesg request, response;

    request.mesg_type = pid;
    request.op = WRITE;

    read_args(p, &request);

    queue_send(clients_server_queue, (char *) &request, mesg_size());

    queue_receive(server_clients_queue, (char *) &response, mesg_size(), pid);

    printf("w %d %d\n", request.args[0], request.args[1]);
}

int main(int argc, char *argv[]) {
    assert(argc == 2);

    long int op_time = strtol(argv[1], NULL, 10);

    assert(op_time >= 0);

    debug("Hello client! Got %ld.\n", op_time);

    control_queue = get_queue(CONTROL_KEY);
    clients_server_queue = get_queue(CLIENTS_SERVER_KEY);
    server_clients_queue = get_queue(SERVER_CLIENTS_KEY);

    pid = getpid();

    debug("My pid is %ld.\n", pid);

    Mesg msg;
    msg.mesg_type = pid;

    queue_send(control_queue, (char *) &msg, mesg_size());

    char buffer[500];
    while (fgets(buffer, sizeof buffer, stdin) != NULL) {
        char op;
        int n;
        const char *p = buffer;

        sscanf(p, "%c%n", &op, &n);
        p += n;

        switch (op) {
            case 'r': op_read(p); break;
            case 'w': op_write(p); break;
        }
    }

    msg.op = QUIT;
    queue_send(clients_server_queue, (char *) &msg, mesg_size());

    return 0;
}