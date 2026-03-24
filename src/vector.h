#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

#define REALLOC_BUFFER 8

typedef struct {
    int *data;
    size_t size;
    size_t alloc_size;
} IntVector_t;

IntVector_t intv_create(size_t alloc_size);
void intv_destroy(IntVector_t vector);
void intv_realloc(IntVector_t *vector, size_t new_size);

void intv_append(IntVector_t *vector, int value);
void intv_pop(IntVector_t *vector);
void intv_erase(IntVector_t *vector);

size_t intv_indexof(IntVector_t vector, int value);

void intv_insert(IntVector_t *vector, int value, size_t index);
void intv_popindex(IntVector_t *vector, size_t index);

void intv_print(IntVector_t vector, int pretty);

#endif//VECTOR_H