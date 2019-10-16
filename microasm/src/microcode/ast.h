#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "shared/memory.h"

struct Token;
struct Parser;

typedef struct BitArray {
    DEFINE_ARRAY(struct Token, data);
} BitArray;

typedef struct Line {
    bool hasCondition;
    BitArray bitsLow;
    BitArray bitsHigh;
    struct Token conditionErrorToken;
} Line;

typedef struct ASTHeader {
    DEFINE_ARRAY(BitArray, line);
    struct Token errorPoint;
} ASTHeader;

typedef struct ASTOpcode {
    struct Token id;
    struct Token name;
    DEFINE_ARRAY(Line*, line);
} ASTOpcode;

typedef struct ASTParameter {
    struct Token name;
    struct Token value;
} ASTParameter;

typedef enum ASTStatementType {
    AST_BLOCK_HEADER,
    AST_BLOCK_OPCODE,
    AST_BLOCK_PARAMETER
} ASTStatementType;

typedef struct ASTStatement {
    ASTStatementType type;

    union {
        ASTHeader header;
        ASTOpcode opcode;
        ASTParameter parameter;
    } as;

    bool isValid;
} ASTStatement;

typedef struct AST {
    DEFINE_ARRAY(const char*, fileName);

    DEFINE_ARRAY(ASTStatement, statement);
} AST;

void InitAST(AST* mcode);

ASTStatement* newStatement(struct Parser* parser, ASTStatementType type);

#endif
