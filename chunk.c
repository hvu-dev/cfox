#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void new_chunk(Chunk *chunk) {
    chunk->capacity = 0;
    chunk->length = 0;
    chunk->code = NULL;
	chunk->lines = NULL;
	clear_pool(&chunk->pool);	
}

void free_chunk(Chunk *chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	free_pool(&chunk->pool);
    new_chunk(chunk);
}

void write_byte_to_chunk(Chunk *chunk, uint8_t byte, int line) {
    if (chunk->length + 1 > chunk->capacity) {
        const int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    }
	
    chunk->code[chunk->length] = byte;
	chunk->lines[chunk->length] = line;
    chunk->length++;
}

int add_constant(Chunk *chunk, Value value) {
	write_value_to_pool(&chunk->pool, value);
	return chunk->pool.length - 1;
}

int get_instruction_line_number(int index) {
	//
}
