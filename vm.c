#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "value.h"

#include "object.h"
#include "vm.h"

static VM vm;
static void reset_stack() { vm.stack_top = vm.stack; }

void init_vm() { reset_stack(); }

void free_vm() {}

Value pop() {
  vm.stack_top--;
  return *vm.stack_top;
}

static Value peek(int distance) { return vm.stack_top[-1 - distance]; }

static bool is_falsy(Value value) {
  return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void make_runtime_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);
  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[Line %d] in script\n", line);

  reset_stack();
}

void concatenate() {
  ObjString *b = AS_STRING(pop());
  ObjString *a = AS_STRING(pop());

  const size_t new_length = a->length + b->length;
  char *new_chars = ALLOCATE(char, new_length + 1);

  memcpy(new_chars, a->chars, a->length);
  memcpy(new_chars + a->length, b->chars, b->length);

  new_chars[new_length] = '\0';

  ObjString *result = take_string(new_chars, new_length);
  push(OBJECT_VAL(result));
}

InterpretResult run() {
#define READ_BYTE() (*vm.ip++) // return uint8_t
#define READ_CONSTANT() (vm.chunk->pool.values[READ_BYTE()])
#define BINARY_OP(value_type, op)                                              \
  do {                                                                         \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                          \
      make_runtime_error("Operands must be numbers");                          \
      return INTERPRETER_RUNTIME_ERROR;                                        \
    }                                                                          \
    double b = AS_NUMBER(pop());                                               \
    double a = AS_NUMBER(pop());                                               \
    push(value_type(a op b));                                                  \
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
    for (Value *slot = vm.stack; slot < vm.stack_top; slot++) {
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
      if (!IS_NUMBER(peek(0))) {
        make_runtime_error("Operand must be a number");
        return INTERPRETER_RUNTIME_ERROR;
      }
      push(NUMBER_VAL(-AS_NUMBER(pop())));
      break;
    case OP_RETURN:
      print_value(pop());
      return INTERPRETER_OK;
    case OP_ADD:
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenate();
      } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());

        push(NUMBER_VAL(a + b));
      } else {
        make_runtime_error("Operands must be strings or numbers");
        return INTERPRETER_RUNTIME_ERROR;
      }
      break;
    case OP_SUBSTRACT:
      BINARY_OP(NUMBER_VAL, -);
      break;
    case OP_MULTIPLY:
      BINARY_OP(NUMBER_VAL, *);
      break;
    case OP_DIVIDE:
      BINARY_OP(NUMBER_VAL, /);
      break;
    case OP_NULL:
      push(NULL_VAL);
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_NOT:
      push(BOOL_VAL(is_falsy(pop())));
      break;
    case OP_EQUAL:
      Value b = pop();
      Value a = pop();
      push(BOOL_VAL(check_equality(a, b)));
      break;
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    }
  }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source) {
  Chunk chunk;
  new_chunk(&chunk);

  if (!compile(source, &chunk)) {
    free_chunk(&chunk);
    return INTERPRETER_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;
  InterpretResult result = run();
  free_chunk(&chunk);
  return result;
}

void push(Value value) {
  *vm.stack_top = value;
  vm.stack_top++;
}
