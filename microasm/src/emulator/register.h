#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

// create a register
void regInit(Register* reg);

// link a register to a bus
unsigned int regConnectBus(VMCore* core, unsigned int reg, unsigned int bus);

// write integer to register - for test purposes only 
void regWriteInt(VMCore* core, unsigned int reg, uint16_t val);

// get a registers value for debug printing
uint16_t regRead(VMCore* core, unsigned int reg);

#endif