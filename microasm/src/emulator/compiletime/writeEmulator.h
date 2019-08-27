#ifndef WRITE_EMULATOR_H
#define WRITE_EMULATOR_H

#include "microcode/token.h"
#include "microcode/ast.h"

void writeEmulator(const char* filename, Microcode* mcode);

#endif