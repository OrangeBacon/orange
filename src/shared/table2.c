#include "shared/table2.h"

// find an entry in a list of entries
static Entry2* findEntry(Entry2* entries, int capacity, Key2* key, CompareFn2 cmp) {

    // location to start searching
    uint32_t index = key->hash % capacity;

    while(true) {
        // value to check
        Entry2* entry = &entries[index];

        // if run out of entries to check or the correct entry is found,
        // return current search item
        if(entry->key.key == NULL || cmp(entry->key.key, key->key)) {
            return entry;
        }

        // get next location, wrapping round to index 0
        index = (index + 1) % capacity;
    }
}

// increase the size of a table to the given capacity
static void adjustCapacity(Table2* table, unsigned int capacity) {

    // create new data section and null initialise
    Entry2* entries = ArenaAlloc(sizeof(Entry2) * capacity);
    for(unsigned int i = 0; i < capacity; i++) {
        entries[i].key.key = NULL;
        entries[i].value = NULL;
    }

    // copy data from the table to the new data section
    // re-organises table for new memory size not just
    // copying the data across.
    for(unsigned int i = 0; i < table->entryCapacity; i++) {
        Entry2* entry = &table->entrys[i];
        if(entry->key.key == NULL) {
            continue;
        }

        Entry2* dest = findEntry(entries, capacity, &entry->key, table->cmp);
        dest->key = entry->key;
        dest->value = entry->value;
    }

    // set the table's data to the new data
    table->entrys = entries;
    table->entryCapacity = capacity;
}

void table2Set(Table2* table, void* keyPtr, void* valuePtr) {
    // create the key to be inserted into the table
    Key2 key;
    key.key = keyPtr;
    key.hash = table->hash(valuePtr);

    // make sure the table is big enough
    if(table->entryCount + 1 > table->entryCapacity * TABLE2_MAX_LOAD) {
        int capacity = table->entryCapacity;
        capacity = capacity < 8 ? 8 : capacity * 2;
        adjustCapacity(table, capacity);
    }

    // get the entry to be set
    Entry2* entry = findEntry(table->entrys, table->entryCapacity, &key, table->cmp);

    // does the entry have any content?
    bool isNewKey = entry->key.key == NULL;
    if(isNewKey) {
        table->entryCount++;
    }

    entry->key = key;
    entry->value = valuePtr;
}

void* table2Get(Table2* table, void* keyPtr) {
    // if nothing has been set then get will always be false
    if(table->entrys == NULL) {
        return false;
    }

    // create key to search for
    Key2 key;
    key.key = keyPtr;
    key.hash = table->hash(keyPtr);

    // find location where the key should be
    Entry2* entry = findEntry(table->entrys, table->entryCapacity, &key, table->cmp);
    if(entry->key.key == NULL) {
        return NULL;
    }

    return &entry->value;
}

bool table2Has(Table2* table, void* key) {
    if(table->entrys == NULL) {
        return false;
    }

    Key2 test;
    test.key = key;
    test.hash = table->hash(key);

    Entry2* entry = findEntry(table->entrys, table->entryCapacity, &test, table->cmp);

    // if the item is in the table, its key will not be null
    return !(entry->key.key == NULL);
}

void table2Remove(Table2* table, void* key) {
    if(table->entrys == NULL) {
        return;
    }

    Key2 test;
    test.key = key;
    test.hash = table->hash(key);

    Entry2* entry = findEntry(table->entrys, table->entryCapacity, &test, table->cmp);
    entry->key.key = NULL;
}