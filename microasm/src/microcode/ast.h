#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "shared/memory.h"

struct Token;
struct Error;

typedef struct Condition {
    struct Token name;
    struct Token value;
} Condition;

typedef struct BitArray {
    DEFINE_ARRAY(struct Token, data);
} BitArray;

typedef struct Line {
    DEFINE_ARRAY(Condition, condition);
    bool anyCondition;
    BitArray bits;
    struct Token conditionErrorToken;
} Line;

typedef struct Header {
    DEFINE_ARRAY(BitArray, line);
    bool isValid;
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
} Input;

typedef struct OpCode {
    struct Token id;
    struct Token name;
    bool isValid;
    DEFINE_ARRAY(struct Token, parameter);
    DEFINE_ARRAY(Line*, line);
} OpCode;

typedef struct AST {
    const char* fileName;

    DEFINE_ARRAY(struct Error, error);
    Header head;
    Input inp;

    DEFINE_ARRAY(OpCode, opcode);
    
#ifdef debug
    DEFINE_ARRAY(struct Error, expectedError);
#endif
} AST;

void InitAST(AST* mcode, const char* fileName);

typedef struct AnalysisAst {
    unsigned int opsize;
} AnalysisAst;

#endif