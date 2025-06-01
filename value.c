#include <stdio.h>
#include <stddef.h>

#include "value.h"
#include "memory.h"

ConstantPool* new_constant_pool() {
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
		pool->values = GROW_ARRAY(Value, pool->values, old_capacity, pool->capacity);	
	}

	pool->values[pool->length] = value;
	pool->length++;
}

void free_pool(ConstantPool *pool){
	FREE_ARRAY(Value, pool->values, pool->capacity);
	clear_pool(pool);
}

void print_value(Value value) {
	printf("%g", value);
}

