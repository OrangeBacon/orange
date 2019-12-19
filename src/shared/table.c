#include <stdbool.h>
#include <string.h>
#include "shared/table.h"
#include "shared/memory.h"

uint32_t strHash(void* value) {
    // fnv-1a hash calculation
    // assumes null-terminated string
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
    // are the strings equal?
    return strcmp(tokA, tokB) == 0;
}

// create new hash table
void initTable(Table* table, HashFn hashfn, KeyCompare cmp) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
    table->hash = hashfn;
    table->cmp = cmp;
}

// find an entry in a list of entries
static Entry* findEntry(Entry* entries, int capacity, Key* key, KeyCompare cmp) {

    // location to start searching
    uint32_t index = key->hash % capacity;

    while(true) {
        // value to check
        Entry* entry = &entries[index];

        // if run out of entries to check or the correct entry is found,
        // return current search item
        if(entry->key.value == NULL || cmp(entry->key.value, key->value)) {
            return entry;
        }

        // get next location, wrapping round to index 0
        index = (index + 1) % capacity;
    }
}

// increase the size of a table to the given capacity
static void adjustCapacity(Table* table, unsigned int capacity) {
    
    // create new data section and null initialise
    Entry* entries = ArenaAlloc(sizeof(Entry) * capacity);
    for(unsigned int i = 0; i < capacity; i++) {
        entries[i].key.value = NULL;
        entries[i].value = NULL;
    }

    // copy data from the table to the new data section
    // re-organises table for new memory size not just
    // copying the data across.
    for(unsigned int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if(entry->key.value == NULL) {
            continue;
        }

        Entry* dest = findEntry(entries, capacity, &entry->key, table->cmp);
        dest->key = entry->key;
        dest->value = entry->value;
    }

    // set the table's data to the new data
    table->entries = entries;
    table->capacity = capacity;
}


bool tableSet(Table* table, void* voidKey, void* value) {
    // create the key to be inserted into the table
    Key key;
    key.value = voidKey;
    key.hash = table->hash(voidKey);

    // make sure the table is big enough
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = table->capacity;
        capacity = capacity < 8 ? 8 : capacity * 2;
        adjustCapacity(table, capacity);
    }

    // get the entry to be set
    Entry* entry = findEntry(table->entries, table->capacity, &key, table->cmp);

    // does the entry have any content?
    bool isNewKey = entry->key.value == NULL;
    if(isNewKey) {
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableGet(Table* table, void* voidKey, void** value) {
    // if nothing has been set then get will always be false
    if(table->entries == NULL) {
        return false;
    }
    
    // create key to search for
    Key key;
    key.value = voidKey;
    key.hash = table->hash(voidKey);

    // find location where the key should be
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

    // if the item is in the table, its key will not be null
    return !(entry->key.value == NULL);
}
