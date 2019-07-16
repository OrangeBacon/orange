#include <stdbool.h>
#include <string.h>
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

bool tokenCmp(void* a, void* b) {
    Token* tokA = a;
    Token* tokB = b;

    if(tokA->length != tokB->length) {
        return false;
    }
    return strncmp(TOKEN_GET(*tokA), TOKEN_GET(*tokB), tokA->length) == 0;
}

uint32_t strHash(void* value) {
    char* str = value;
    uint32_t hash = 2166126261u;

    for(size_t i = 0; i < strlen(str); i++) {
        hash ^= str[i];
        hash *= 16777619;
    }

    return hash;
}

bool strCmp(void* a, void* b) {
    char* tokA = a;
    char* tokB = b;

    return strcmp(tokA, tokB) == 0;
}

void initTable(Table* table, HashFn hashfn, KeyCompare cmp) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
    table->hash = hashfn;
    table->cmp = cmp;
}

static Entry* findEntry(Entry* entries, int capacity, Key* key, KeyCompare cmp) {
    uint32_t index = key->hash % capacity;

    for(;;) {
        Entry* entry = &entries[index];

        if(entry->key.value == NULL || cmp(entry->key.value, key->value)) {
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

        Entry* dest = findEntry(entries, capacity, &entry->key, table->cmp);
        dest->key = entry->key;
        dest->value = entry->value;
    }

    table->entries = entries;
    table->capacity = capacity;
}


bool tableSet(Table* table, void* voidKey, void* value) {
    Key key;
    key.value = voidKey;
    key.hash = table->hash(voidKey);

    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = table->capacity;
        capacity = capacity < 8 ? 8 : capacity * 2;
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, &key, table->cmp);

    bool isNewKey = entry->key.value == NULL;
    if(isNewKey) {
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableGet(Table* table, void* voidKey, void** value) {
    if(table->entries == NULL) {
        return false;
    }
    
    Key key;
    key.value = voidKey;
    key.hash = table->hash(voidKey);

    Entry* entry = findEntry(table->entries, table->capacity, &key, table->cmp);
    if(entry->key.value == NULL) {
        return false;
    }

    *value = entry->value;
    return true;
}

bool tableGetKey(Table* table, void* voidKey, void** realkey) {
    if(table->entries == NULL) {
        return false;
    }

    Key key;
    key.value = voidKey;
    key.hash = table->hash(voidKey);

    Entry* entry = findEntry(table->entries, table->capacity, &key, table->cmp);
    *realkey = entry->key.value;

    return entry->key.value != NULL;
}

bool tableHas(Table* table, void* key) {
    if(table->entries == NULL) {
        return false;
    }
    
    Key test;
    test.value = key;
    test.hash = table->hash(key);

    Entry* entry = findEntry(table->entries, table->capacity, &test, table->cmp);
    if(entry->key.value == NULL) {
        return false;
    }

    return true;
}