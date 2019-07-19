#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

void regInit(Register* reg);
void regConnectBus(RegisterId* reg, BusId* bus);
void regWriteInt(RegisterId* reg, uint16_t val);
uint16_t regRead(RegisterId* reg);

void regSetBus(RegisterId* reg, BusId* bus);
void regReadBus(RegisterId* reg, BusId* bus);

#endif