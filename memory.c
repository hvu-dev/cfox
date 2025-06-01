#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

void *reallocate(void *pointer, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void *allocated_mem = realloc(pointer, new_size);
    if (allocated_mem == NULL) {
        printf("Insufficient memory");
        exit(1);
    };

    return allocated_mem;
}
