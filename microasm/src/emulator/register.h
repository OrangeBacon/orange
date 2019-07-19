#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

void regConnectBus(VMCore* core, RegisterId* reg, BusId* bus);
void regWriteInt(RegisterId* reg, uint16_t val);

#endif