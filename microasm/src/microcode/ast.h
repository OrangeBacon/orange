#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "shared/memory.h"

struct Token;
struct Error;

typedef struct BitArray {
    DEFINE_ARRAY(struct Token, data);
} BitArray;

typedef struct Line {
    bool hasCondition;
    BitArray bitsLow;
    BitArray bitsHigh;
    struct Token conditionErrorToken;
} Line;

typedef struct Header {
    DEFINE_ARRAY(BitArray, line);
    bool isValid;
    bool isPresent;
    struct Token errorPoint;
} Header;

typedef struct InputValue {
    struct Token name;
    struct Token value;
} InputValue;

typedef struct Input {
    DEFINE_ARRAY(InputValue, value);
    struct Token inputHeadToken;
    bool isValid;
    bool isPresent;
} Input;

typedef struct OpCode {
    struct Token id;
    struct Token name;
    bool isValid;
    DEFINE_ARRAY(Line*, line);
} OpCode;

typedef struct AST {
    DEFINE_ARRAY(const char*, fileName);

    DEFINE_ARRAY(struct Error, error);
    Header head;
    Input inp;

    DEFINE_ARRAY(OpCode, opcode);
    
#ifdef debug
    DEFINE_ARRAY(struct Error, expectedError);
#endif
} AST;

void InitAST(AST* mcode);

#endif
