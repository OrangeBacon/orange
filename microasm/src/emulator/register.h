#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

void regInit(Register* reg);
void regConnectBus(VMCore* core, unsigned int reg, unsigned int bus);
void regWriteInt(VMCore* core, unsigned int reg, uint16_t val);
uint16_t regRead(VMCore* core, unsigned int reg);

void regSetBus(VMCore* core, unsigned int reg, unsigned int bus);
void regReadBus(VMCore* core, unsigned int reg, unsigned int bus);

#endif