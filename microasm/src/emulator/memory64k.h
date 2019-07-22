#ifndef MEMORY64K_H
#define MEMORY64K_H

#include <stdint.h>
#include "vmcore.h"

typedef struct Memory64k {
    uint16_t value[2<<15];
} Memory64k;

unsigned int memoryInit(Memory64k* mem, VMCore* core, unsigned int addressBus, unsigned int dataBus);

#endif