#ifndef AST_H
#define AST_H

#include "memory.h"

struct Token;
struct Error;

typedef struct Condition {
    struct Token name;
    unsigned int value;
} Condition;

typedef struct Line {
    DEFINE_ARRAY(Condition, condition);
    bool anyCondition;
    DEFINE_ARRAY(struct Token, bit);
    struct Token conditionErrorToken;
} Line;

typedef struct Header {
    DEFINE_ARRAY(struct Token, bit);
} Header;

typedef struct InputValue {
    struct Token name;
    unsigned int value;
} InputValue;

typedef struct Input {
    DEFINE_ARRAY(InputValue, value);
} Input;

typedef struct OutputValue {
    unsigned int id;
    struct Token name;
} OutputValue;

typedef struct Output {
    DEFINE_ARRAY(OutputValue, value);
    unsigned int width;
} Output;

typedef struct OpCode {
    unsigned int id;
    struct Token name;
    DEFINE_ARRAY(struct Token, parameter);
    DEFINE_ARRAY(Line*, line);
} OpCode;

typedef struct Microcode {
    const char* fileName;
    bool hasError;
    DEFINE_ARRAY(struct Error, error);

    Header head;
    Input inp;
    Output out;
    DEFINE_ARRAY(OpCode, opcode);
#ifdef debug
    DEFINE_ARRAY(struct Error, expectedError);
#endif
} Microcode;

void InitMicrocode(Microcode* mcode, const char* fileName);

void PrintMicrocode(Microcode* mcode);

void FreeMicrocode(Microcode* mcode);

#endif