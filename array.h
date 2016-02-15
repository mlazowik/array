#ifndef ARRAY_ARRAY_H
#define ARRAY_ARRAY_H

void array_init(size_t size);

void lock_read(size_t index);
void unlock_read(size_t index);

void lock_write(size_t index);
void unlock_write(size_t index);

int get_value(size_t index);
void set_value(size_t index, int value);

#endif //ARRAY_ARRAY_H
