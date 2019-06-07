#ifndef AST_H
#define AST_H

#include "token.h"
#include "memory.h"

typedef struct Condition {
    Token name;
    Token value;
} Condition;

typedef struct Line {
    Condition* conditions;
    unsigned int conditionCount;
    unsigned int conditionCapacity;
    Token* bits;
    unsigned int bitCount;
    unsigned int bitCapacity;
    Token condition1Equals;
} Line;

typedef struct Header {
    Token* bits;
    unsigned int bitCount;
    unsigned int bitCapacity;
} Header;

typedef struct Input {
    Token* names;
    unsigned int* values;
    unsigned int count;
    unsigned int capacity;
} Input;

typedef struct Microcode {
    const char* fileName;
    Header head;
    Input inp;
    bool hasError;
    Arena arena;
} Microcode;

void InitMicrocode(Microcode* mcode, const char* fileName);

void PrintMicrocode(Microcode* mcode);

void FreeMicrocode(Microcode* mcode);

#endif