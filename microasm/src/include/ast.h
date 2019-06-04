#ifndef AST_H
#define AST_H

#include "token.h"

typedef struct Condition {
    Token* name;
    Token* value;
} Condition;

typedef struct Line {
    Condition** conditions;
    Token** bits;
    Token* condition1Equals;
} Line;

typedef struct Header {
    Line* line;
} Header;

typedef struct Microcode {
    Header* head;
} Microcode;


void PrintMicrocode(Microcode* mcode);

void FreeMicrocode(Microcode* mcode);

#endif