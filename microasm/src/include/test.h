#ifndef TEST_H
#define TEST_H

#include "scanner.h"
#include "parser.h"

void runFile(const char* fileName, const char* file, Parser* parse, Scanner* scan);

void runTests(const char* directory);

#endif