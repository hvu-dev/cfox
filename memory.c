#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

void *reallocate(void *pointer, size_t old_size, size_t new_size) {
  if (new_size == 0) {
    free(pointer);
    return NULL;
  }

  void *allocated_mem = realloc(pointer, new_size);
  if (allocated_mem == NULL) {
    printf("Insufficient memory");
    exit(1);
  };

  return allocated_mem;
}

static void free_object(FoxObj *obj) {
  switch (obj->type) {
  case OBJ_STRING:
    ObjString *str = (ObjString *)obj;
    FREE_ARRAY(char, str->chars, str->length + 1);
    FREE(ObjString, obj);
    break;
  }
}

void free_objects() {
  FoxObj *obj = vm.objects;
  while (obj != NULL) {
    FoxObj *next = obj->next;
    free_object(next);
    obj = next;
  }
}
