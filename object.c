//
// Created by Huy Vu on 18/8/25.
//

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
// #include "value.h"

#define ALLOCATE_OBJ(type, object_type)                                        \
  (type *)allocate_object(sizeof(type), object_type)

static FoxObj *allocate_object(size_t size, ObjType type) {
  FoxObj *obj = (FoxObj *)reallocate(NULL, 0, size);
  obj->type = type;

  return obj;
}

static ObjString *allocate_string(char *chars, int length) {
  ObjString *obj = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  obj->chars = chars;
  obj->length = length;

  return obj;
}

ObjString *copy_string(const char *chars, int length) {
  char *copied_string = ALLOCATE(char, length + 1);
  memcpy(copied_string, chars, length);
  copied_string[length] = '\0';
  return allocate_string(copied_string, length);
}

ObjString *take_string(char *chars, int length) {
  return allocate_string(chars, length);
}

void print_object(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    printf("%s\n", AS_CSTRING(value));
    break;
  }
}
