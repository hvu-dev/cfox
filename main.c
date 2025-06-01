#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;
    new_chunk(&chunk);
	int constant = add_constant(&chunk, 1.2);

	write_byte_to_chunk(&chunk, OP_CONSTANT, 123);
	write_byte_to_chunk(&chunk, constant, 123);
    write_byte_to_chunk(&chunk, OP_RETURN, 123);
	
    disassemble_chunk(&chunk, "test chunk");
    free_chunk(&chunk);
    return 0;
}
