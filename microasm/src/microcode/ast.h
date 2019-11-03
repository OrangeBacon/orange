#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "shared/memory.h"

struct Token;
struct Parser;

typedef struct Bit {
    struct Token data;
    DEFINE_ARRAY(struct Token, param);
} Bit;

typedef struct BitArray {
    DEFINE_ARRAY(Bit, data);
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

typedef struct ASTParameter {
    struct Token name;
    struct Token value;
} ASTParameter;

typedef struct ASTOpcode {
    struct Token id;
    struct Token name;
    DEFINE_ARRAY(Line*, line);
    DEFINE_ARRAY(ASTParameter, param);
} ASTOpcode;

typedef struct ASTTypeEnum {
    struct Token width;
    DEFINE_ARRAY(struct Token, member);
} ASTTypeEnum;

typedef struct ASTType {
    struct Token name;

    enum {
        AST_BLOCK_TYPE_ENUM
    } type;

    union {
        ASTTypeEnum enumType;
    } as;
} ASTType;

typedef struct ASTBitGroupIdentifier {
    enum {
        AST_BIT_GROUP_IDENTIFIER_SUBST,
        AST_BIT_GROUP_IDENTIFIER_LITERAL
    } type;
    struct Token identifier;
} ASTBitGroupIdentifier;

typedef struct ASTBitGroup {
    struct Token name;
    DEFINE_ARRAY(ASTParameter, param);
    DEFINE_ARRAY(ASTBitGroupIdentifier, segment);
} ASTBitGroup;

typedef enum ASTStatementType {
    AST_BLOCK_HEADER,
    AST_BLOCK_OPCODE,
    AST_BLOCK_PARAMETER,
    AST_BLOCK_TYPE,
    AST_BLOCK_BITGROUP
} ASTStatementType;

typedef struct ASTStatement {
    ASTStatementType type;

    union {
        ASTHeader header;
        ASTOpcode opcode;
        ASTParameter parameter;
        ASTType type;
        ASTBitGroup bitGroup;
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
