#include <stdbool.h>
#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"

#include "vm.h"

static VM vm;
static void reset_stack() { vm.stackTop = vm.stack; }

void init_vm() { reset_stack(); }

void free_vm() {}

InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // return uint8_t
#define READ_CONSTANT() (vm.chunk->pool.values[READ_BYTE()])
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double b = pop();                                                          \
    double a = pop();                                                          \
    push(a op b);                                                              \
  } while (false)
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    // pointer arithmetic
    // 	  0		 1		2		3
    // |======|======|======|======|
    // 	  ^				 ^
    // 	 code  			ip
    // => ip - code = offset (e.g: 2 - 0 = 2)
    printf("\n");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[");
      print_value(*slot);
      printf("]");
    }
    disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) { // instruction = OP_CODE
    case OP_CONSTANT:
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    case OP_NEGATE:
      push(-pop());
      break;
    case OP_RETURN:
      print_value(pop());
      return INTERPRETER_OK;
    case OP_ADD:
      BINARY_OP(+);
      break;
    case OP_SUBSTRACT:
      BINARY_OP(-);
      break;
    case OP_MULTIPLY:
      BINARY_OP(*);
      break;
    case OP_DIVIDE:
      BINARY_OP(/);
      break;
    }
  }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source) {
  compile(source);
  return run();
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}
