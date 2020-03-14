#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "microcode/token.h"
#include "shared/memory.h"

struct Parser;

// ----------- //
// expressions //
// ----------- //

typedef enum ASTExpressionType {
    AST_EXPRESSION_NUMBER,
    AST_EXPRESSION_BINARY,
    AST_EXPRESSION_UNARY,
    AST_EXPRESSION_CALL,
    AST_EXPRESSION_VARIABLE,
    AST_EXPRESSION_STRING,
    AST_EXPRESSION_LIST,
} ASTExpressionType;

typedef enum ASTExpressionBinaryType {
    AST_EXPRESSION_BINARY_OR,
    AST_EXPRESSION_BINARY_AND,
    AST_EXPRESSION_BINARY_EQUAL,
    AST_EXPRESSION_BINARY_NOT_EQUAL,
} ASTExpressionBinaryType;

typedef enum ASTExpressionUnaryType {
    AST_EXPRESSION_UNARY_NOT,
} ASTExpressionUnaryType;

typedef struct ASTExpression {
    ASTExpressionType type;

    union {
        Token number;
        struct {
            ASTExpressionBinaryType type;
            Token opcode;
            struct ASTExpression* left;
            struct ASTExpression* right;
        } binary;
        struct {
            ASTExpressionUnaryType type;
            Token opcode;
            struct ASTExpression* operand;
        } unary;
        struct {
            struct ASTExpression* callee;
            ARRAY_DEFINE(struct ASTExpression*, param);
        } call;
        Token variable;
        Token string;
        struct {
            ARRAY_DEFINE(struct ASTExpression*, element);
        } list;
    } as;
} ASTExpression;


// ---------- //
// Statements //
// ---------- //

typedef struct ASTStatementHeader {
    ARRAY_DEFINE(ASTExpression*, expression);
    Token errorPoint;
    SourceRange range;
} ASTStatementHeader;

typedef struct ASTStatementParameter {
    Token name;
    ASTExpression* value;
    SourceRange range;
} ASTStatementParameter;

typedef struct ASTFunctionParameter {
    Token name;
    Token value;
    SourceRange range;
} ASTFunctionParameter;

typedef struct ASTStatementOpcode {
    Token id;
    Token name;
    ARRAY_DEFINE(ASTExpression*, expression);
    ARRAY_DEFINE(ASTFunctionParameter, param);
    SourceRange range;
} ASTStatementOpcode;

typedef struct ASTTypeEnum {
    Token width;
    ARRAY_DEFINE(Token, member);
    SourceRange range;
} ASTTypeEnum;

typedef enum ASTTypeStatementType {
    AST_TYPE_STATEMENT_ANY,
    AST_TYPE_STATEMENT_ENUM,
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
    ARRAY_DEFINE(ASTFunctionParameter, param);
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

void PrintAST(AST* ast);

#endif
