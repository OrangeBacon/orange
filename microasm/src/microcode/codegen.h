#ifndef CODEGEN_H
#define CODEGEN_H

#include "microcode/token.h"
#include "microcode/ast.h"

void codegen(Microcode* m, char* outputFile);

#endif