#ifndef CODEGEN_H
#define CODEGEN_H

#include "emulator/compiletime/create.h"

void addCoreLoop(VMCoreGen* core, Parser* mcode);

void writeCore(VMCoreGen* core, const char* filename);

#endif