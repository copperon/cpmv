#include "vector.h"
#include <stdio.h>
#include <string.h>

IntVector_t intv_create(size_t alloc_size) {
    IntVector_t vector;
    vector.data = malloc(alloc_size * sizeof(int));
    vector.alloc_size = alloc_size;
    vector.size = 0;
    
    return vector;
}

void intv_destroy(IntVector_t vector) {
    free(vector.data);
    vector.data = NULL;
}

void intv_realloc(IntVector_t *vector, size_t new_size) {
    vector->data = realloc(vector->data, new_size * sizeof(int));
    vector->alloc_size = new_size;
}

void intv_append(IntVector_t *vector, int value) {
    // vector grew outside original size, reallocate data
    if (vector->size == vector->alloc_size) {
        intv_realloc(vector, vector->size + REALLOC_BUFFER);
    }
    vector->data[vector->size] = value;
    vector->size++;
}

void intv_pop(IntVector_t *vector) {
    if (vector->size > 0) {
        vector->size--;
    }
}

void intv_erase(IntVector_t *vector) {
    vector->size = 0;
}

size_t intv_indexof(IntVector_t vector, int value) {
    for (size_t i = 0; i < vector.size; i++) {
        if (vector.data[i] == value) {
            return i;
        }
    }
    return vector.size;
}

void intv_insert(IntVector_t *vector, int value, size_t index) {
    // vector grew outside original size, reallocate data
    if (vector->size + 1 >= vector->alloc_size) {
        intv_realloc(vector, vector->size + REALLOC_BUFFER);
    }

    if (vector->size != 0) {
        for (size_t i = vector->size - 1; i >= index; i--) {
            vector->data[i+1] = vector->data[i];
        }
    }
    vector->size++;
    vector->data[index] = value;
}

void intv_popindex(IntVector_t *vector, size_t index) {
    if (vector->size == 0) {
        return;
    }

    for (int i = index; i < vector->size-1; i++) {
        vector->data[i] = vector->data[i + 1];
    }
    vector->size--;
}

void intv_print(IntVector_t vector, int pretty) {
    if (vector.size == 0) {
        printf("[]\n");
        return;
    }
    
    int pad_size = 1;
    if (pretty) {
        // get padding size
        for (size_t i = 0; i < vector.size; i++) {
            int size;
            char buf[10];
            sprintf(buf, "%zu", i);
            if ((size = strlen(buf)) > pad_size)
                pad_size = size;
        }
    }
    
    printf("[%*d", pad_size, vector.data[0]);
    for (size_t i = 1; i < vector.size; i++) {
        if (pretty && i % 8 == 0) {
            printf(",\n %*d", pad_size, vector.data[i]);
        } else {
            printf(", %*d", pad_size, vector.data[i]);
        }
    }
    printf("]\n");
}