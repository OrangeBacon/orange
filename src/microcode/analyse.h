#ifndef ANALYSE_H
#define ANALYSE_H

#include "microcode/parser.h"
#include "emulator/compiletime/create.h"

void Analyse(Parser* parser, VMCoreGen* core);

#endif
