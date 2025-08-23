#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

ConstantPool *new_constant_pool() {
  ConstantPool *pool;
  pool->values = NULL;
  pool->capacity = 0;
  pool->length = 0;

  return pool;
}

void clear_pool(ConstantPool *pool) {
  pool->values = NULL;
  pool->capacity = 0;
  pool->length = 0;
}

void write_value_to_pool(ConstantPool *pool, Value value) {
  if (pool->capacity < pool->length + 1) {
    int old_capacity = pool->capacity;
    pool->capacity = GROW_CAPACITY(old_capacity);
    pool->values =
        GROW_ARRAY(Value, pool->values, old_capacity, pool->capacity);
  }

  pool->values[pool->length] = value;
  pool->length++;
}

void free_pool(ConstantPool *pool) {
  FREE_ARRAY(Value, pool->values, pool->capacity);
  clear_pool(pool);
}

void print_value(Value value) {
  switch (value.type) {
  case VAL_NUMBER:
    printf("%g", AS_NUMBER(value));
    break;
  case VAL_NULL:
    printf("null");
    break;
  case VAL_BOOL:
    printf("%s", AS_BOOL(value) ? "true" : "false");
    break;
  }
}

bool check_equality(Value a, Value b) {
  if (a.type != b.type)
    return false;
  switch (a.type) {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NUMBER:
    return AS_NUMBER(a) == AS_NUMBER(b);
  case VAL_OBJECT:
    switch (OBJ_TYPE(a)) {
    case OBJ_STRING:
      const ObjString *aString = AS_STRING(a);
      const ObjString *bString = AS_STRING(b);
      /* There is another way to compare string using strcmp
       * However it will not take size into account, for example:
       * char a[]  = {'a', 'b', 'c', '\0'}; // explicitly add another null
       * terminitor
       * char b[]  = {'a', 'b', 'c'};
       * printf("%zu", strlen(a)) // print "3"
       * printf("%zu", strlen(b)) // print "3"
       * printf("%zu", sizeof(a)) // print "4"
       * printf("%zu", sizeof(b)) // print "3"
       * printf("%s", strcmp(a, b) == 0 ? "true" : "false"); // print "true"
       * See this example on SO for more details:
       * https://stackoverflow.com/a/13095574
       * */
      return aString->length == bString->length &&
             memcmp(aString->chars, bString->chars, aString->length) == 0;
    }
  case VAL_NULL:
    return true;
  default:
    return false;
  }
}
