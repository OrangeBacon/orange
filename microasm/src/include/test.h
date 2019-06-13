#ifndef TEST_H
#define TEST_H

#include "scanner.h"
#include "parser.h"

bool runFile(const char* fileName, const char* file, Parser* parse, Scanner* scan, bool testing);

void runTests(const char* directory);

#endif