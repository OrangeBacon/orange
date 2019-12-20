#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// percentage full that a table should expand at
#define TABLE_MAX_LOAD 0.75

// wrapper around key object
typedef struct Key {
    // pointer to object being used as key e.g. token, string
    void* value;

    // hash of the void* pointer in the key
    uint32_t hash;
} Key;

// hash table key-value pair
typedef struct Entry {
    Key key;
    void* value;
} Entry;

// get hash of key, used to hash different types
typedef uint32_t(*HashFn)(void*);

// are the two objects the same? (e.g. strcmp)
typedef bool(*KeyCompare)(void*, void*);

// Hash table
typedef struct Table {
    // number of items in the table
    unsigned int count;

    // maximum number of items in the table
    unsigned int capacity;

    // how should the keys be hashed
    HashFn hash;

    // how should the keys be compared
    KeyCompare cmp;

    // the key-value array
    Entry* entries;
} Table;

// hash function for const char* keys
uint32_t strHash(void* value);

// key comparison function for const char* keys
bool strCmp(void* a, void* b);

// initialise a table
void initTable(Table* table, HashFn hash, KeyCompare cmp);

// set a key-value pair in the table
bool tableSet(Table* table, void* key, void* value);

// get a value given a key, void** value is set the the item
// retrieved from the table
bool tableGet(Table* table, void* key, void** value);

// given a key, find the key-value pair it coresponds to
// and return a pointer to the key in the table, used if
// not all data in the key is compared, so data can be stored
// in the key
bool tableGetKey(Table* table, void* voidKey, void** realkey);

// does the key exist in the table
bool tableHas(Table* table, void* key);

void tableRemove(Table* table, void* key);

#endif
