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

long pid, op_time;

int get_queue(key_t key) {
    int id;

    id = msgget(key, 0);

    if (id == -1) {
        syserr("Cannot get queue");
    }

    return id;
}

void queue_send(int id, void *msg) {
    if (msgsnd(id, msg, mesg_size(), 0) != 0) {
        syserr("Cannot send to queue");
    }
}

void queue_receive(int id, void *msg, long type) {
    if (msgrcv(id, msg, mesg_size(), type, 0) <= 0) {
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

void send_request(Mesg *request, Mesg *response) {
    queue_send(clients_server_queue, (char *) request);
    queue_receive(server_clients_queue, (char *) response, pid);
}

void op_read(const char *p) {
    Mesg request, response;

    request.mesg_type = pid;
    request.op = READ;

    read_args(p, &request);

    send_request(&request, &response);

    printf("r %d %d\n", request.args[0], response.args[0]);
}

void op_write(const char *p) {
    Mesg request, response;

    request.mesg_type = pid;
    request.op = WRITE;

    read_args(p, &request);

    send_request(&request, &response);

    printf("w %d %d\n", request.args[0], request.args[1]);
}

void op_sum(const char *p) {
    Mesg request, response;

    request.mesg_type = pid;
    request.op = SUM_GET;

    read_args(p, &request);

    send_request(&request, &response);

    sleep(op_time);

    request.op = SUM_SET;

    int sum = 0;
    size_t i;
    for (i = 1; i < request.args_count; i++) {
        sum += response.args[i];
    }

    request.args[request.args_count++] = sum;

    send_request(&request, &response);

    printf("s %d", request.args[0]);
    for (i = 1; i < request.args_count; i++) {
        printf(" %d", request.args[i]);
    }
    printf("\n");
}

void op_swap(const char *p) {
    Mesg request, response;

    request.mesg_type = pid;
    request.op = SWAP_GET;

    read_args(p, &request);

    send_request(&request, &response);

    sleep(op_time);

    request.op = SWAP_SET;

    send_request(&request, &response);

    printf("x %d %d\n", request.args[0], request.args[1]);
}

int main(int argc, char *argv[]) {
    assert(argc == 2);

    op_time = strtol(argv[1], NULL, 10);

    assert(op_time >= 0);

    debug("Hello client! Got %ld.\n", op_time);

    control_queue = get_queue(CONTROL_KEY);
    clients_server_queue = get_queue(CLIENTS_SERVER_KEY);
    server_clients_queue = get_queue(SERVER_CLIENTS_KEY);

    pid = getpid();

    debug("My pid is %ld.\n", pid);

    Mesg msg;
    msg.mesg_type = pid;

    queue_send(control_queue, (char *) &msg);

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
            case 's': op_sum(p); break;
            case 'x': op_swap(p); break;
        }
    }

    msg.op = QUIT;
    queue_send(clients_server_queue, (char *) &msg);

    return 0;
}