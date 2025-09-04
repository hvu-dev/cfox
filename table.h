#ifndef TABLE_H
#define TABLE_H
#include <stdint.h>

#include "value.h"

typedef struct {
  ObjString *key;
  Value value;
} Entry;

typedef struct {
  unsigned int capacity;
  unsigned int length;
  Entry *entries;
} Table;

void init_table(Table *table);
void free_table(Table *table);
bool get_entry(Table *table, ObjString *key, Value *value);
bool set_entry(Table *table, ObjString *key, Value value);
bool delete_entry(Table *table, ObjString *key);
void transfer_entries(Table *from, Table *to);
ObjString *find_string(Table *from, const char *chars, int length,
                       uint32_t hash);

#endif
