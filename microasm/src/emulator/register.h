#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

void regInit(Register* reg);
unsigned int regConnectBus(VMCore* core, unsigned int reg, unsigned int bus);
void regWriteInt(VMCore* core, unsigned int reg, uint16_t val);
uint16_t regRead(VMCore* core, unsigned int reg);

#endif