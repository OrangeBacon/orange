#ifndef WRITE_EMULATOR_H
#define WRITE_EMULATOR_H

#include "microcode/parser.h"
#include "emulator/compiletime/vmcoregen.h"

void createEmulator(VMCoreGen* core);
void writeEmulator(Parser* parser, VMCoreGen* core, const char* filename);

#endif