#ifndef CODEGEN_H
#define CODEGEN_H

#include "emulator/compiletime/create.h"

void coreLinkAnalysisResult(VMCoreGen* core, Parser* mcode, AnalysisAst* ast);

void coreCodegen(VMCoreGen* core, const char* filename);

#endif