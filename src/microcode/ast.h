#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "microcode/token.h"
#include "shared/memory.h"

struct Parser;

typedef struct Bit {
    Token data;
    DEFINE_ARRAY(Token, param);
} Bit;

typedef struct BitArray {
    DEFINE_ARRAY(Bit, data);
} BitArray;

typedef struct Line {
    bool hasCondition;
    BitArray bitsLow;
    BitArray bitsHigh;
    Token conditionErrorToken;
} Line;

typedef struct ASTHeader {
    DEFINE_ARRAY(BitArray, line);
    Token errorPoint;
} ASTHeader;

typedef struct ASTParameter {
    Token name;
    Token value;
} ASTParameter;

typedef struct ASTOpcode {
    Token id;
    Token name;
    DEFINE_ARRAY(Line*, line);
    DEFINE_ARRAY(ASTParameter, param);
} ASTOpcode;

typedef struct ASTTypeEnum {
    Token width;
    DEFINE_ARRAY(Token, member);
} ASTTypeEnum;

typedef enum UserType {
    USER_TYPE_ANY,
    USER_TYPE_ENUM
} UserType;

typedef struct ASTType {
    Token name;

    UserType type;

    union {
        ASTTypeEnum enumType;
    } as;
} ASTType;

typedef struct ASTBitGroupIdentifier {
    enum {
        AST_BIT_GROUP_IDENTIFIER_SUBST,
        AST_BIT_GROUP_IDENTIFIER_LITERAL
    } type;
    Token identifier;
} ASTBitGroupIdentifier;

typedef struct ASTBitGroup {
    Token name;
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
