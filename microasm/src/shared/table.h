#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TABLE_MAX_LOAD 0.75

typedef struct Key {
    void* value;
    uint32_t hash;
} Key;

typedef struct Entry {
    Key key;
    void* value;
} Entry;

typedef uint32_t(*HashFn)(void*);
typedef bool(*KeyCompare)(void*, void*);

typedef struct Table {
    int count;
    int capacity;
    HashFn hash;
    KeyCompare cmp;
    Entry* entries;
} Table;

uint32_t strHash(void* value);
bool strCmp(void* a, void* b);
void initTable(Table* table, HashFn hash, KeyCompare cmp);
bool tableSet(Table* table, void* key, void* value);
bool tableGet(Table* table, void* key, void** value);
bool tableGetKey(Table* table, void* voidKey, void** realkey);
bool tableHas(Table* table, void* key);

#endif