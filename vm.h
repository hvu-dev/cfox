#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "table.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
  Chunk *chunk;
  uint8_t *ip; // instruction pointer (program counter): the instruction is
               // going to be executed NEXT
  Value stack[STACK_MAX];
  Value *stack_top; // points at the "next" value of the stack, not the
                    // currently being used one
  Table strings;
  FoxObj *objects;
} VM;

extern VM vm;

typedef enum {
  INTERPRETER_OK,
  INTERPRETER_COMPILE_ERROR,
  INTERPRETER_RUNTIME_ERROR
} InterpretResult;

void init_vm();
void free_vm();
InterpretResult interpret(const char *source);
InterpretResult run();

void push(Value value);
Value pop();
#endif // VM_H
