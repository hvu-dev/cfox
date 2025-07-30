#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void start_repl() {
  char line[1024];
  while(true) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line);
  }
}

static char *run_file(const char *file_path) {
  FILE *file = fopen(file_path, "rb");
  fseek(file, 0L, SEEK_END);

  size_t file_size = ftell(file);
  rewind(file);
  char *buffer = (char*) malloc(file_size + 1);
  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  buffer[bytes_read] = '\0';

  fclose(file);

  return buffer;
}

int main(int argc, const char *argv[]) {
  init_vm();

  if(argc == 1) {
    start_repl();
  } else if (argc == 2) {
    run_file(argv[1])
  } else {
    fprintf(stderr, "Invalid numbers of arguments");
    exit(64);
  }

  free_vm();
  return 0;
}
