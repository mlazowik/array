#ifndef ARRAY_MESG_H
#define ARRAY_MESG_H

#define	MAX_INTS 9

typedef enum {
    QUIT,
    WRITE,
    READ,
    SUM_GET,
    SUM_SET,
    SWAP_GET,
    SWAP_SET
} op_type;

typedef struct {
    long	mesg_type;
    op_type op;
    int args_count;
    int args[MAX_INTS];
} Mesg;

#define CONTROL_KEY 1234L
#define CLIENTS_SERVER_KEY 1235L
#define SERVER_CLIENTS_KEY 1236L

size_t mesg_size() {
    return sizeof(Mesg) - sizeof(long);
}

#endif //ARRAY_MESG_H
