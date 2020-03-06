#ifndef TABLE2_H
#define TABLE2_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "shared/memory.h"
#include "shared/log.h"

#define TABLE2_MAX_LOAD 0.75

// wrapper around key object
typedef struct Key2 {
    // pointer to object being used as key e.g. token, string
    void* key;

    // hash of the data in the key
    uint32_t hash;
} Key2;

typedef struct Entry2 {
    Key2 key;
    void* value;
} Entry2;

// get hash of key, used to hash different types
typedef uint32_t(*HashFn2)(void*);

// are the two objects the same? (e.g. strcmp)
typedef bool(*CompareFn2)(void*, void*);

typedef struct Table2 {
    // how should the keys be hashed
    HashFn2 hash;

    // how should the keys be compared
    CompareFn2 cmp;

    // hash table
    ARRAY_DEFINE(Entry2, entry);

#ifdef DEBUG_BUILD
    const char* keyType;
    const char* valueType;
#endif
} Table2;

#define TABLE2_STRING(x) #x

#ifdef DEBUG_BUILD
#define TABLE2_INIT(table, hashFn, cmpFn, kType, vType) \
    do { \
        table.keyType = TABLE2_STRING(typeof(kType)); \
        table.valueType = TABLE2_STRING(typeof(vType)); \
        ARRAY_ALLOC(Entry2, (table), entry); \
        table.hash = (hashFn); \
        table.cmp = (cmpFn); \
    } while(0)
#else
#define TABLE2_INIT(table, hashFn, cmpFn, keyType, valueType) \
    do { \
        ARRAY_ALLOC(Entry2, table, entry); \
        table.hash = hashFn; \
        table.cmp = cmpFn; \
    } while(0)
#endif

void table2Set(Table2* table, void* key, void* value);
void* table2Get(Table2* table, void* key);
bool table2Has(Table2* table, void* key);
void table2Remove(Table2* table, void* key);

#ifdef DEBUG_BUILD
#define TABLE_SET(table, key, value) \
    do { \
        if(strcmp(TABLE2_STRING(typeof(key)), (table).keyType) != 0) { \
            WARN("Seting table member based on incorrect key type, correct " \
                "is %s, got %s", (table).keyType, \
                TABLE2_STRING(typeof(key))); \
        } \
        if(strcmp(TABLE2_STRING(typeof(value)), (table).valueType) != 0) { \
            WARN("Seting table member based on incorrect value type, correct " \
                "is %s, got %s", (table).valueType, \
                TABLE2_STRING(typeof(value))); \
        } \
        table2Set(&table, (void*)key, (void*)value); \
    } while(0)
#else
#define TABLE_SET(table, key, value) \
    table2Set(&(table), (void*)(key), (void*)(value))
#endif

#ifdef DEBUG_BUILD
#define TABLE_GET(table, key) \
    ({ \
        if(strcmp(TABLE2_STRING(typeof(key)), (table).keyType) != 0) { \
            WARN("Getting table member based on incorrect key type, correct " \
                "is %s, got %s", (table).keyType, \
                TABLE2_STRING(typeof(key))); \
        } \
        table2Get(&(table), (void*)key); \
    })
#else
#define TABLE_GET(table, key) \
    table2Get(&(table), (void*)key)
#endif

#ifdef DEBUG_BUILD
#define TABLE_HAS(table, key) \
    ({ \
        if(strcmp(TABLE2_STRING(typeof(key)), (table).keyType) != 0) { \
            WARN("Checking table member based on incorrect key type, correct " \
                "is %s, got %s", (table).keyType, \
                TABLE2_STRING(typeof(key))); \
        } \
        table2Has(&(table), (void*)key); \
    })
#else
#define TABLE_HAS(table, key) \
    table2Has(&(table), (void*)key)
#endif

#ifdef DEBUG_BUILD
#define TABLE_REMOVE(table, key) \
    do { \
        if(strcmp(TABLE2_STRING(typeof(key)), (table).keyType) != 0) { \
            WARN("Removing table member based on incorrect key type, correct " \
                "is %s, got %s", (table).keyType, \
                TABLE2_STRING(typeof(key))); \
        } \
        table2Remove(&(table), (void*)key); \
    } while(0)
#else
#define TABLE_REMOVE(table, key) \
    table2Remove(&(table), (void*)key)
#endif

// hash function for const char* keys
uint32_t hashstr(void* value);

// key comparison function for const char* keys
bool cmpstr(void* a, void* b);


#endif