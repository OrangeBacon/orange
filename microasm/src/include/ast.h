#ifndef AST_H
#define AST_H

#include "token.h"
#include "memory.h"

typedef struct Condition {
    Token name;
    Token value;
} Condition;

typedef struct Line {
    DEFINE_ARRAY(Condition, condition);
    DEFINE_ARRAY(Token, bit);
    Token condition1Equals;
} Line;

typedef struct Header {
    DEFINE_ARRAY(Token, bit);
} Header;

typedef struct InputValue {
    Token name;
    unsigned int value;
} InputValue;

typedef struct Input {
    DEFINE_ARRAY(InputValue, value);
} Input;

typedef struct OutputValue {
    unsigned int id;
    Token name;
} OutputValue;

typedef struct Output {
    DEFINE_ARRAY(OutputValue, value);
    unsigned int width;
} Output;

typedef struct Microcode {
    const char* fileName;
    bool hasError;
    
    Header head;
    Input inp;
    Output out;
} Microcode;

void InitMicrocode(Microcode* mcode, const char* fileName);

void PrintMicrocode(Microcode* mcode);

void FreeMicrocode(Microcode* mcode);

#endif