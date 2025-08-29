#ifndef OBJECT_H
#define OBJECT_H

#include "value.h"

#define OBJ_TYPE(value) (AS_OBJECT(value)->type)
#define IS_STRING(value) is_object_type(value, OBJ_STRING)
#define AS_STRING(value) ((ObjString *)AS_OBJECT(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJECT(value))->chars)

typedef enum {
  OBJ_STRING,
} ObjType;

struct FoxObj {
  ObjType type;
  struct FoxObj *next;
};

struct ObjString {
  FoxObj obj;
  int length;
  char *chars;
};

ObjString *copy_string(const char *chars, int length);
ObjString *take_string(char *chars, int length);

void print_object(Value value);

// Inline function body gets copied to the caller function on call
// Rather than pushing to the call stack which may create overhead
static inline bool is_object_type(Value value, ObjType type) {
  return IS_OBJECT(value) && OBJ_TYPE(value) == type;
}
#endif // OBJECT_H
