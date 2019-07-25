#ifndef VMCORE_H
#define VMCORE_H

#include <stdarg.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

void vmcoreInit(VMCore* core);

unsigned int createBus(VMCore* core);
unsigned int createRegister(VMCore* core);
void coreCall(VMCore* core, unsigned int method);
void coreCallLine(VMCore* core, unsigned int count, ...);

#endif