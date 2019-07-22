#ifndef VMCORE_H
#define VMCORE_H

#include "shared/memory.h"
#include "emulator/emulator.h"

void vmcoreInit(VMCore* core);

unsigned int createBus(VMCore* core);
unsigned int createRegister(VMCore* core);

#endif