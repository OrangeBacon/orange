#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

typedef struct Area {
    size_t bytesLeft;
    void* end;
    void* base;
} Area;

typedef struct Arena {
    Area* areas;
    size_t arenaCount;
    size_t pageSize;
    size_t align;
} Arena;

// setup a new arena
void ArenaInit(Arena* arena);

// allocate a new section of memory
void* ArenaAlloc(Arena* arena, size_t size);

void* ArenaAllocAlign(Arena* arena, size_t size, size_t align);

void ArenaFree(Arena* arena);

#endif