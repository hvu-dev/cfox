#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

typedef struct FoxObj FoxObj;
typedef struct ObjString ObjString;

/* Using tagged union to represent a "Value" and its type
 * A union in C will store multiple values in the same memory address which will
 * take X bytes, where X is the type has the largest memory size.
 * Despite it may represent multiple types, but only one can be accessed at a
 * time. Assigning a value will overwrite any previous defined value in a union
 *
 * Ex: the following Union will take 8 bytes (float data size)
 * union Data {
 *    bool a;
 *    int b;
 * };
 * union Data d;
 * d.a = true;
 * printf("%d", d.a); // print 1
 * d.b = 4;
 * printf("%d", d.b); // print 4
 * printf("%d", d.a); // print 0 - d.a got overrided
 * */
typedef enum {
  VAL_BOOL,
  VAL_NULL,
  VAL_NUMBER,
  VAL_OBJECT,
} ValueType;

// This Value struct will take 16 bytes
// 4 bytes ValueType (enum is a constant with int type)
// 4 bytes memory padding (depends on platform and compiler)
// 8 bytes union (double takes 8 bytes)
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    FoxObj *obj;
  } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NULL(value) ((value).type == VAL_NULL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJECT(value) ((value).type == VAL_OBJECT)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJECT(value) ((value).as.obj)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NULL_VAL ((Value){VAL_NULL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJECT_VAL(object) ((Value){VAL_OBJECT, {.obj = (FoxObj *)object}})

typedef struct {
  int capacity;
  int length;
  Value *values;
} ConstantPool;

ConstantPool *new_constant_pool();
void clear_pool(ConstantPool *pool);
void write_value_to_pool(ConstantPool *pool, Value value);
void free_pool(ConstantPool *pool);
void print_value(Value value);
bool check_equality(Value a, Value b);
#endif
