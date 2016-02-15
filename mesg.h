#ifndef ARRAY_MESG_H
#define ARRAY_MESG_H

#define	MAX_INTS 9

typedef struct {
    long	mesg_type;
    int ints[MAX_INTS];
} Mesg;

#define	MKEY	1234L

size_t mesg_size() {
    return sizeof(Mesg) - sizeof(long);
}

#endif //ARRAY_MESG_H