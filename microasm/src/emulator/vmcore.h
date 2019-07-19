#ifndef VMCORE_H
#define VMCORE_H

#include "shared/memory.h"
#include "emulator/emulator.h"

void vmcoreInit(VMCore* core);

BusId createBus(VMCore* core);
RegisterId createRegister(VMCore* core);

#endif