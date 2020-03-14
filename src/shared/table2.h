#ifndef TABLE2_H
#define TABLE2_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "shared/memory.h"
#include "shared/log.h"

#define TABLE2_MAX_LOAD 0.75

typedef struct Key2 {
    char* key;

    uint32_t hash;
} Key2;

typedef struct Entry2 {
    Key2 key;

    void* value;
} Entry2;

typedef struct Table2 {
    // hash table
    ARRAY_DEFINE(Entry2, entry);

#ifdef DEBUG_BUILD
    unsigned int valueSize;
#endif
} Table2;

#define TABLE2_STRING(x) #x

void adjustCapacity(Table2* table, unsigned int capacity);

#ifdef DEBUG_BUILD
#define TABLE2_INIT(table, vType) \
    do { \
        table.valueSize = sizeof(vType); \
        ARRAY_ZERO(table, entry); \
        adjustCapacity(&(table), 8); \
    } while(0)
#else
#define TABLE2_INIT(table, vType) \
    do { \
        ARRAY_ZERO(table, entry); \
        adjustCapacity(&(table), 8); \
    } while(0)
#endif

void table2Set(Table2* table, char* key, void* value);
void* table2Get(Table2* table, char* key);
bool table2Has(Table2* table, char* key);
void table2Remove(Table2* table, char* key);

#ifdef DEBUG_BUILD
#define TABLE2_SET(table, key, value) \
    do { \
        if(sizeof(value) != (table).valueSize) { \
            WARN("Seting table member based on incorrect value size, correct " \
                "is %u, got %u", (table).valueSize, sizeof(value)); \
        } \
        table2Set(&(table), (char*)(key), (void*)(value)); \
    } while(0)
#else
#define TABLE2_SET(table, key, value) \
    table2Set(&(table), (char*)(key), (void*)(value))
#endif

#define TABLE2_GET(table, key) \
    table2Get(&(table), (char*)(key))

#define TABLE2_HAS(table, key) \
    table2Has(&(table), (char*)(key))

#define TABLE2_REMOVE(table, key) \
    table2Remove(&(table), (char*)(key))

#endif