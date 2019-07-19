#ifndef TEST_H
#define TEST_H

#include "microcode/scanner.h"
#include "microcode/parser.h"

bool runFileName(const char* fileName);
bool runFile(const char* fileName, const char* file, Parser* parse, bool testing);

#ifdef debug
void runTests(const char* directory);
#endif

#endif