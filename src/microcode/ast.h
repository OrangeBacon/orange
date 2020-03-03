#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "microcode/token.h"
#include "shared/memory.h"

struct Parser;

typedef struct ASTBitParameter {
    Token name;
} ASTBitParameter;

typedef struct ASTBit {
    Token data;
    ARRAY_DEFINE(ASTBitParameter, param);
    SourceRange range;
} ASTBit;

typedef struct ASTBitArray {
    ARRAY_DEFINE(ASTBit, data);
    SourceRange range;
} ASTBitArray;

typedef struct ASTMicrocodeLine {
    bool hasCondition;
    ASTBitArray bitsLow;
    ASTBitArray bitsHigh;
    Token conditionErrorToken;
    SourceRange range;
} ASTMicrocodeLine;

typedef struct ASTStatementHeader {
    ARRAY_DEFINE(ASTBitArray, line);
    Token errorPoint;
    SourceRange range;
} ASTStatementHeader;

typedef struct ASTStatementParameter {
    Token name;
    Token value;
    SourceRange range;
} ASTStatementParameter;

typedef struct ASTStatementOpcode {
    Token id;
    Token name;
    ARRAY_DEFINE(ASTMicrocodeLine*, line);
    ARRAY_DEFINE(ASTStatementParameter, param);
    SourceRange range;
} ASTStatementOpcode;

typedef struct ASTTypeEnum {
    Token width;
    ARRAY_DEFINE(Token, member);
    SourceRange range;
} ASTTypeEnum;

typedef enum ASTTypeStatementType {
    AST_TYPE_STATEMENT_ANY,
    AST_TYPE_STATEMENT_ENUM
} ASTTypeStatementType;

typedef struct ASTStatementType {
    Token name;

    ASTTypeStatementType type;

    union {
        ASTTypeEnum enumType;
    } as;
    SourceRange range;
} ASTStatementType;

typedef enum AstBitGroupIdentifierType {
    AST_BIT_GROUP_IDENTIFIER_SUBST,
    AST_BIT_GROUP_IDENTIFIER_LITERAL
} AstBitGroupIdentifierType;

typedef struct ASTBitGroupIdentifier {
    AstBitGroupIdentifierType type;
    Token identifier;
    SourceRange range;
} ASTBitGroupIdentifier;

typedef struct ASTStatementBitGroup {
    Token name;
    ARRAY_DEFINE(ASTStatementParameter, param);
    ARRAY_DEFINE(ASTBitGroupIdentifier, segment);
    SourceRange range;
} ASTStatementBitGroup;

typedef enum ASTStatementBlockType {
    AST_BLOCK_HEADER,
    AST_BLOCK_OPCODE,
    AST_BLOCK_PARAMETER,
    AST_BLOCK_TYPE,
    AST_BLOCK_BITGROUP
} ASTStatementBlockType;

typedef struct ASTStatement {
    ASTStatementBlockType type;

    union {
        ASTStatementHeader header;
        ASTStatementOpcode opcode;
        ASTStatementParameter parameter;
        ASTStatementType type;
        ASTStatementBitGroup bitGroup;
    } as;

    bool isValid;
} ASTStatement;

typedef struct AST {
    ARRAY_DEFINE(const char*, fileName);

    ARRAY_DEFINE(ASTStatement, statement);
} AST;

void InitAST(AST* mcode);

ASTStatement* newStatement(struct Parser* parser, ASTStatementBlockType type);

#endif
