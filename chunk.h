#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#include "value.h"

typedef enum {
	OP_CONSTANT,
    OP_RETURN,
} OpCode;

typedef struct {
    int length;
    int capacity;
    uint8_t *code;
	int *lines;
	ConstantPool pool;
} Chunk;

void new_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);
void write_byte_to_chunk(Chunk *chunk, uint8_t byte, int line);
int add_constant(Chunk *chunk, Value value);

#endif
