#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char *argv[]) {
  init_vm();

  Chunk chunk;
  new_chunk(&chunk);

  int constant = add_constant(&chunk, 1.2);
  write_byte_to_chunk(&chunk, OP_CONSTANT, 123);
  write_byte_to_chunk(&chunk, constant, 123);
  write_byte_to_chunk(&chunk, OP_ADD, 123);

  constant = add_constant(&chunk, 5.6);
  write_byte_to_chunk(&chunk, OP_CONSTANT, 123);
  write_byte_to_chunk(&chunk, constant, 123);
  write_byte_to_chunk(&chunk, OP_DIVIDE, 123);

  constant = add_constant(&chunk, 2);
  write_byte_to_chunk(&chunk, OP_CONSTANT, 123);
  write_byte_to_chunk(&chunk, constant, 123);

  write_byte_to_chunk(&chunk, OP_NEGATE, 123);
  write_byte_to_chunk(&chunk, OP_RETURN, 123);
  disassemble_chunk(&chunk, "test");
  interpret(&chunk);
  free_vm();
  free_chunk(&chunk);
  return 0;
}
