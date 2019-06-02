#include <stdio.h>
#include <string.h>
#include "token.h"
#include "scanner.h"

void ScannerInit(Scanner* scanner, const char* source) {
    scanner->line = 1;
    scanner->column = 1;
    scanner->current = source;
    scanner->start = source;
}