#include <stdbool.h>
#include "table.h"
#include "memory.h"

// FNV-1a
uint32_t tokenHash(void* value) {
    Token* token = value;
    uint32_t hash = 2166126261u;

    for(int i = 0; i < token->length; i++) {
        hash ^= TOKEN_GET(*token)[i];
        hash *= 16777619;
    }

    return hash;
}

void initTable(Table* table, HashFn hashfn) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
    table->hash = hashfn;
}

static Entry* findEntry(Entry* entries, int capacity, Key* key) {
    uint32_t index = key->hash % capacity;

    for(;;) {
        Entry* entry = &entries[index];

        if(entry->key.value == key->value || entry->key.value == NULL) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table* table, int capacity) {
    Entry* entries = ArenaAlloc(sizeof(Entry) * capacity);
    for(int i = 0; i < capacity; i++) {
        entries[i].key.value = NULL;
        entries[i].value = NULL;
    }

    for(int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if(entry->key.value == NULL) {
            continue;
        }

        Entry* dest = findEntry(entries, capacity, &entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
    }

    table->entries = entries;
    table->capacity = capacity;
}


bool tableSet(Table* table, void* key, void* value) {
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = table->capacity;
        capacity = capacity < 8 ? 8 : capacity * 2;
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);

    bool isNewKey = entry->key.value == NULL;
    if(isNewKey) {
        table->count++;
    }

    entry->key.value = key;
    entry->key.hash = table->hash(key);
    entry->value = value;
    return isNewKey;
}

bool tableGet(Table* table, void* key, void** value) {
    if(table->entries == NULL) {
        return false;
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if(entry->key.value == NULL) {
        return false;
    }

    *value = entry->value;
    return true;
}