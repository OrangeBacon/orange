#ifndef MEMORY64K_H
#define MEMORY64K_H

#include <stdint.h>
#include "vmcore.h"

// 2^16 addresses of 16 bit
// = 128kb
// = 64k words
typedef struct Memory64k {
    uint16_t value[2<<15];
} Memory64k;

// initialise a memory segment for the vm
unsigned int memoryInit(Memory64k* mem, VMCore* core, unsigned int addressBus, unsigned int dataBus);

#endif