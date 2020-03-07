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
    unsigned int keyType;
    unsigned int valueType;
#endif
} Table2;

#define TABLE2_STRING(x) #x

void adjustCapacity(Table2* table, unsigned int capacity);

#ifdef DEBUG_BUILD
#define TABLE2_INIT(table, hashFn, cmpFn, kType, vType) \
    do { \
        table.keyType = sizeof(kType); \
        table.valueType = sizeof(vType); \
        table.hash = (hashFn); \
        table.cmp = (cmpFn); \
        ARRAY_ZERO(table, entry); \
        adjustCapacity(&(table), 8); \
    } while(0)
#else
#define TABLE2_INIT(table, hashFn, cmpFn, keyType, valueType) \
    do { \
        table.hash = hashFn; \
        table.cmp = cmpFn; \
        ARRAY_ZERO(table, entry); \
        adjustCapacity(&(table), 8); \
    } while(0)
#endif

void table2Set(Table2* table, void* key, void* value);
void* table2Get(Table2* table, void* key);
bool table2Has(Table2* table, void* key);
void table2Remove(Table2* table, void* key);

#ifdef DEBUG_BUILD
#define TABLE2_SET(table, key, value) \
    do { \
        if(sizeof(key) != (table).keyType) { \
            WARN("Seting table member based on incorrect key size, correct " \
                "is %u, got %u", (table).keyType, sizeof(key)); \
        } \
        if(sizeof(value) != (table).valueType) { \
            WARN("Seting table member based on incorrect value size, correct " \
                "is %u, got %u", (table).valueType, sizeof(value)); \
        } \
        table2Set(&table, (void*)key, (void*)value); \
    } while(0)
#else
#define TABLE2_SET(table, key, value) \
    table2Set(&(table), (void*)(key), (void*)(value))
#endif

#ifdef DEBUG_BUILD
#define TABLE2_GET(table, key) \
    __extension__ ({ \
        if(sizeof(key) != (table).keyType) { \
            WARN("Getting table member based on incorrect key size, correct " \
                "is %u, got %u", (table).keyType, sizeof(key)); \
        } \
        table2Get(&(table), (void*)key); \
    })
#else
#define TABLE2_GET(table, key) \
    table2Get(&(table), (void*)key)
#endif

#ifdef DEBUG_BUILD
#define TABLE2_HAS(table, key) \
    __extension__ ({ \
        if(sizeof(key) != (table).keyType) { \
            WARN("Checking table member based on incorrect key size, correct " \
                "is %u, got %u", (table).keyType, sizeof(key)); \
        } \
        table2Has(&(table), (void*)key); \
    })
#else
#define TABLE2_HAS(table, key) \
    table2Has(&(table), (void*)key)
#endif

#ifdef DEBUG_BUILD
#define TABLE2_REMOVE(table, key) \
    do { \
        if(sizeof(key) != (table).keyType) { \
            WARN("Removing table member based on incorrect key size, correct " \
                "is %u, got %u", (table).keyType, sizeof(key)); \
        } \
        table2Remove(&(table), (void*)key); \
    } while(0)
#else
#define TABLE2_REMOVE(table, key) \
    table2Remove(&(table), (void*)key)
#endif

// hash function for const char* keys
uint32_t hashstr(void* value);

// key comparison function for const char* keys
bool cmpstr(void* a, void* b);


#endif