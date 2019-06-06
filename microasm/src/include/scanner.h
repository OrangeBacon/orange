#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

// scanner current source infomation
typedef struct Scanner {
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

#endif