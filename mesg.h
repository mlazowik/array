#ifndef ARRAY_MESG_H
#define ARRAY_MESG_H

#define	MAX_INTS 9

typedef struct {
    long	mesg_type;
    int ints[MAX_INTS];
} Mesg;

#define	CONTROL_KEY	1234L
#define CLIENTS_KEY 1235L

size_t mesg_size() {
    return sizeof(Mesg) - sizeof(long);
}

#endif //ARRAY_MESG_H
