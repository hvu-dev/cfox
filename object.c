//
// Created by Huy Vu on 18/8/25.
//

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type)                                        \
  (type *)allocate_object(sizeof(type), object_type)

/* FNV-1a hash function:
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 * For each byte in data, performs an XOR operation with the current hash value
 * Then multiply the (hash value) result with a fixed FNV prime numer
 * */
static uint32_t hash_string(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }

  return hash;
}

static FoxObj *allocate_object(size_t size, ObjType type) {
  FoxObj *obj = (FoxObj *)reallocate(NULL, 0, size);
  obj->type = type;
  obj->next = vm.objects;
  vm.objects = obj;
  return obj;
}

static ObjString *allocate_string(char *chars, int length, uint32_t hash) {
  ObjString *obj = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  obj->chars = chars;
  obj->length = length;
  obj->hash = hash;

  return obj;
}

ObjString *copy_string(const char *chars, int length) {
  uint32_t hashed_chars = hash_string(chars, length);
  char *copied_string = ALLOCATE(char, length + 1);
  memcpy(copied_string, chars, length);
  copied_string[length] = '\0';
  return allocate_string(copied_string, length, hashed_chars);
}

ObjString *take_string(char *chars, int length) {
  uint32_t hashed_chars = hash_string(chars, length);
  return allocate_string(chars, length, hashed_chars);
}

void print_object(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    printf("%s\n", AS_CSTRING(value));
    break;
  }
}
