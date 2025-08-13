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

static char *read_file(const char *file_path) {
  FILE *file = fopen(file_path, "rb");

  if(file == NULL) {
    fprintf(stderr, "Could not open file '%s'", file_path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = (char*) malloc(file_size + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Memory is not sufficient to read file '%s'", file_path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if(bytes_read < file_size) {
    fprintf(stderr, "Could not read file '%s'", file_path);
    exit(74);
  }
  buffer[bytes_read] = '\0';

  fclose(file);

  return buffer;
}

static void run_file(const char *file_path){
  char *source = read_file(file_path);
  InterpretResult result = interpret(source);

  if(result == INTERPRETER_COMPILE_ERROR) exit(65);
  if(result == INTERPRETER_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char *argv[]) {
  init_vm();

  if(argc == 1) {
    start_repl();
  } else if (argc == 2) {
    run_file(argv[1]);
  } else {
    fprintf(stderr, "Invalid numbers of arguments");
    exit(64);
  }

  free_vm();
  return 0;
}
