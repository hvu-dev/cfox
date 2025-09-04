#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

static Entry *find_entry(Entry *entries, int capacity, ObjString *key) {
  uint32_t index = key->hash % capacity;
  Entry *tombstone = NULL;
  while (true) {
    Entry *entry = &entries[index];
    if (entry->key == NULL) {
      if (IS_NULL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else if (tombstone == NULL)
        tombstone = entry;
    } else {
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

bool get_entry(Table *table, ObjString *key, Value *value) {
  if (table->length == 0)
    return false;

  Entry *entry = find_entry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

static void adjust_capacity(Table *table, int new_capacity) {
  Entry *entries = ALLOCATE(Entry, new_capacity);
  for (int i = 0; i < new_capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NULL_VAL;
  }

  table->length = 0;
  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL) {
      continue;
    }

    Entry *dest = find_entry(entries, new_capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->length++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = new_capacity;
}

void transfer_entries(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL) {
      set_entry(to, entry->key, entry->value);
    }
  }
}

void init_table(Table *table) {
  table->capacity = 0;
  table->length = 0;
  table->entries = NULL;
}

void free_table(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  init_table(table);
}

bool set_entry(Table *table, ObjString *key, Value value) {
  if (table->length + 1 > table->capacity * TABLE_MAX_LOAD) {
    int new_capacity = GROW_CAPACITY(table->capacity);
    adjust_capacity(table, new_capacity);
  }
  Entry *entry = find_entry(table->entries, table->capacity, key);
  bool is_new_key = entry->key == NULL;
  if (is_new_key && IS_NULL(entry->value))
    table->length++;

  return is_new_key;
}

bool delete_entry(Table *table, ObjString *key) {
  if (table->length == 0)
    return false;
  Entry *entry = find_entry(table->entries, table->length, key);
  if (entry->key == NULL)
    return false;

  entry->key = NULL;
  entry->value = BOOL_VAL(true);
  return true;
}

ObjString *find_string(Table *from, const char *chars, int length,
                       uint32_t hash) {
  if (from->length == 0)
    return NULL;

  uint32_t index = hash % from->capacity;
  while (true) {
    Entry *entry = &from->entries[index];
    if (entry->key == NULL && IS_NULL(entry->value))
      return NULL;
    if (entry->key->length == length && entry->key->hash == hash &&
        memcmp(entry->key->chars, chars, length) == 0)
      return entry->key;
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

    index = (index + 1) % from->capacity;
  }
}
