#ifndef TEST_H
#define TEST_H

#include "microcode/token.h"
#include "microcode/ast.h"

bool runFileName(const char* fileName);
bool runFile(const char* fileName, const char* file, AST* ast);

#endif
