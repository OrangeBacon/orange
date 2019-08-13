#ifndef VMCORE_H
#define VMCORE_H

#include <stdarg.h>
#include "shared/memory.h"
#include "emulator/emulator.h"

// create an emulator
void vmcoreInit(VMCore* core);

void vmcoreFree(VMCore* core);

// add a bus to the emulator, return its id
unsigned int createBus(VMCore* core, const char* name);

// add a register to the emulator, return its id
unsigned int createRegister(VMCore* core, const char* name);

// call a microcode command given its id
void coreCall(VMCore* core, unsigned int method);

// run one clock cycle of the emulator, where the given microcode
// commands are executed.  The commands will be sorted based on their
// dependancies and what the change to ensure data flows in the correct
// direction, electricaly all commands would run simultaneously
void coreCallLine(VMCore* core, unsigned int count, ...);

#endif