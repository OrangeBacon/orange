#ifndef SCANNER_H
#define SCANNER_H

// scanner current source infomation
typedef struct Scanner {
    const char* current;
    const char* start;
    int line;
    int column;
} Scanner;

// initialise (or re-initialise) an already existing scanner
void ScannerInit(Scanner* scanner, const char* source);

#endif