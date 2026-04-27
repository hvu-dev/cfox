#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_RETURN,
    OP_NEGATE,
    OP_PRINT,
    OP_ADD,
    OP_SUBSTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
} OpCode;

typedef struct {
    int length;
    int capacity;
    uint8_t *code;
    int *lines;
    ConstantPool pool;
} Chunk;

typedef struct {
    int occurrence;
    int line_number;
} Line;

void new_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);
void write_byte_to_chunk(Chunk *chunk, uint8_t byte, int line);
int add_constant(Chunk *chunk, Value value);
int get_line_number_by_instruction_index(int index);

#endif
