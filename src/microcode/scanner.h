#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>
#include "microcode/token.h"

// scanner current source infomation
typedef struct Scanner {
    const char* base;
    const char* fileName;
    const char* current;
    const char* start;
    int line;
    int column;
} Scanner;

// initialise (or re-initialise) an already existing scanner
void ScannerInit(Scanner* scanner, const char* source, const char* fileName);

// get the next token from a scanner
Token ScanToken(Scanner* scanner);

// get the start position and the length of the
// nth line in a string, by iteration (eg O(N))
// returns whether the line was found
// if not, start and length might be invalid
// sets start and length to output values
bool getLine(const char* string, int line, int* start, int* length);

#endif
