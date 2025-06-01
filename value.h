#ifndef VALUE_H
#define VALUE_H

typedef double Value;
typedef struct {
	int capacity;
	int length;
	Value *values;
} ConstantPool;

ConstantPool* new_constant_pool();
void clear_pool(ConstantPool *pool);
void write_value_to_pool(ConstantPool *pool, Value value);
void free_pool(ConstantPool *pool);
void print_value(Value value);

#endif
