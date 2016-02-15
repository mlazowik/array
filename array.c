#include <stdlib.h>
#include <pthread.h>

#include "err.h"
#include "array.h"

pthread_mutex_t mutex;

typedef struct {
    int value, reading, writing;
    pthread_cond_t read_lock, write_lock;
    int waiting_read, waiting_write;
} Cell;

static Cell *array;

static void lock() {
    int err;

    if ((err = pthread_mutex_lock(&mutex)) != 0) {
        syserr_errno(err, "Lock failed");
    }
}

static void unlock() {
    int err;

    if ((err = pthread_mutex_unlock(&mutex)) != 0) {
        syserr_errno(err, "unlock failed");
    }
}

static void wait(pthread_cond_t *cond) {
    int err;

    if ((err = pthread_cond_wait(cond, &mutex)) != 0) {
        syserr_errno(err, "cond wait failed");
    }
}

static void signal(pthread_cond_t *cond) {
    int err;

    if ((err = pthread_cond_signal(cond)) != 0) {
        syserr_errno(err, "cond wait failed");
    }
}

void lock_read(size_t index) {
    lock();

    Cell *cell = &array[index];

    cell->waiting_read++;
    while (cell->writing > 0 || cell->waiting_write > 0) {
        wait(&cell->read_lock);
    }
    cell->waiting_read--;
    cell->reading++;

    signal(&cell->read_lock);

    unlock();
}

void unlock_read(size_t index) {
    lock();

    Cell *cell = &array[index];

    cell->reading--;

    if (cell->reading == 0) {
        signal(&cell->write_lock);
    }

    unlock();
}

void lock_write(size_t index) {
    lock();

    Cell *cell = &array[index];

    cell->waiting_write++;
    while (cell->writing > 0 || cell->reading > 0) {
        wait(&cell->write_lock);
    }
    cell->waiting_write--;
    cell->writing++;

    unlock();
}

int get_value(size_t index) {
    return array[index].value;
}

void set_value(size_t index, int value) {
    array[index].value = value;
}

void unlock_write(size_t index) {
    lock();

    Cell *cell = &array[index];

    cell->writing--;
    if (cell->waiting_read > 0) {
        signal(&cell->read_lock);
    } else {
        signal(&cell->write_lock);
    }

    unlock();
}

void array_init(size_t size) {
    int err;

    if ((array = malloc(sizeof(Cell) * size)) == NULL) {
        fatal("Cannot allocate memory.\n");
    }

    if ((err = pthread_mutex_init(&mutex, 0) != 0)) {
        syserr_errno(err, "mutex init failed");
    }

    for (size_t i = 0; i < size; i++) {
        array[i].value = 0;
        array[i].reading = 0;
        array[i].writing = 0;
        array[i].waiting_read = 0;
        array[i].waiting_write = 0;

        if ((err = pthread_cond_init(&array[i].read_lock, 0) != 0)) {
            syserr_errno(err, "Cond init failed");
        }

        if ((err = pthread_cond_init(&array[i].write_lock, 0) != 0)) {
            syserr_errno(err, "Cond init failed");
        }
    }
}