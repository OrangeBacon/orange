#include "microcode/token.h"
#include "microcode/ast.h"
#include "shared/memory.h"
#include "shared/log.h"
#include "microcode/parser.h"
#include <stdio.h>

void InitAST(AST* mcode) {
    ARRAY_ALLOC(const char*, *mcode, fileName);
    ARRAY_ALLOC(ASTStatement, *mcode, statement);
}

ASTStatement* newStatement(Parser* parser, ASTStatementBlockType type) {
    CONTEXT(DEBUG, "Creating new statement");
    if(parser->ast->statementCount == parser->ast->statementCapacity) {
        parser->ast->statements = ArenaReAlloc(parser->ast->statements,
            sizeof(ASTStatement) * parser->ast->statementCapacity,
            sizeof(ASTStatement) * parser->ast->statementCapacity * 2);
        parser->ast->statementCapacity *= 2;
    }
    ASTStatement* ret = &parser->ast->statements[parser->ast->statementCount];
    parser->ast->statementCount++;
    ret->type = type;
    return ret;
}

static void PrintTabs(int depth) {
    putchar('\n');
    for(int i = 0; i < depth; i++) putchar('\t');
}

static void PrintExpression(ASTExpression* e, int depth);

static void PrintBinaryExpression(ASTExpression* e, int depth) {
    PrintTabs(depth);
    printf("Binary ");
    switch(e->as.binary.type) {
        case AST_EXPRESSION_BINARY_OR:
            printf("or"); break;
        case AST_EXPRESSION_BINARY_AND:
            printf("and"); break;
        case AST_EXPRESSION_BINARY_EQUAL:
            printf("equal"); break;
        case AST_EXPRESSION_BINARY_NOT_EQUAL:
            printf("not equal"); break;
    }
    printf(" expression: ");
    PrintTabs(depth + 1);
    printf("opcode: ");
    TokenPrint(&e->as.binary.opcode);
    printf("\n");
    PrintTabs(depth + 1);
    printf("left: ");
    PrintExpression(e->as.binary.left, depth + 2);
    printf("\n");
    PrintTabs(depth + 1);
    printf("right: ");
    PrintExpression(e->as.binary.right, depth + 2);
    printf("\n");
}

static void PrintUnaryExpression(ASTExpression* e, int depth) {
    PrintTabs(depth);
    printf("Unary ");
    switch(e->as.unary.type) {
        case AST_EXPRESSION_UNARY_NOT:
            printf("not");
    }
    printf(" expression");
    PrintTabs(depth + 1);
    printf("opcode: ");
    TokenPrint(&e->as.unary.opcode);
    printf("\n");
    PrintTabs(depth + 1);
    printf("operand: ");
    PrintExpression(e->as.unary.operand, depth + 2);
    printf("\n");
}

static void PrintCallExpression(ASTExpression* e, int depth) {
    PrintTabs(depth);
    printf("Call expression");
    PrintTabs(depth + 1);
    printf("callee: ");
    PrintExpression(e->as.call.callee, depth + 2);
    PrintTabs(depth + 1);
    printf("parameters: ");
    for(unsigned int i = 0; i < e->as.call.paramCount; i++) {
        PrintExpression(e->as.call.params[i], depth + 2);
    }
    printf("\n");
}

static void PrintListExpression(ASTExpression* e, int depth) {
    PrintTabs(depth);
    printf("List expression: ");
    for(unsigned int i = 0; i < e->as.list.elementCount; i++) {
        PrintExpression(e->as.list.elements[i], depth + 1);
    }
    printf("\n");
}

static void PrintExpression(ASTExpression* e, int depth) {
    if(e == NULL) return;
    switch(e->type) {
        case AST_EXPRESSION_NUMBER:
            PrintTabs(depth);
            printf("Number expression:");
            PrintTabs(depth + 1);
            TokenPrint(&e->as.number);
            break;
        case AST_EXPRESSION_BINARY:
            PrintBinaryExpression(e, depth);
            break;
        case AST_EXPRESSION_UNARY:
            PrintUnaryExpression(e, depth);
            break;
        case AST_EXPRESSION_CALL:
            PrintCallExpression(e, depth);
            break;
        case AST_EXPRESSION_VARIABLE:
            PrintTabs(depth);
            printf("Variable expression:");
            PrintTabs(depth + 1);
            TokenPrint(&e->as.variable);
            break;
        case AST_EXPRESSION_STRING:
            PrintTabs(depth);
            printf("String expression:");
            PrintTabs(depth + 1);
            TokenPrint(&e->as.string);
            break;
        case AST_EXPRESSION_LIST:
            PrintListExpression(e, depth);
            break;
        case AST_EXPRESSION_BOOLEAN:
            PrintTabs(depth);
            printf("Boolean expression:");
            PrintTabs(depth + 1);
            TokenPrint(&e->as.boolean);
            break;
    }
}

static void PrintHeader(ASTStatementHeader* h) {
    printf("\tHeader Statement:\n\t\terror point: ");
    TokenPrint(&h->errorPoint);
    printf("\n\t\tlines: ");
    for(unsigned int i = 0; i < h->expressionCount; i++) {
        PrintExpression(h->expressions[i], 3);
    }
}

static void PrintOpcode(ASTStatementOpcode* o) {
    printf("\tOpcode Statement:\n\t\tid: ");
    TokenPrint(&o->id);
    printf("\n\t\tname: ");
    TokenPrint(&o->name);
    printf("\n\t\tparameters:");
    for(unsigned int i = 0; i < o->paramCount; i++) {
        printf("\n\t\t\tname: ");
        TokenPrint(&o->params[i].name);
        printf("\n\t\t\tvalue: ");
        TokenPrint(&o->params[i].value);
    }
    printf("\n\t\tlines: ");
    for(unsigned int i = 0; i < o->expressionCount; i++) {
        PrintExpression(o->expressions[i], 3);
    }
    printf("\n");
}

static void PrintParameter(ASTStatementParameter* p) {
    printf("\tParameter Statement:\n\t\tname: ");
    TokenPrint(&p->name);
    printf("\n\t\tvalue: ");
    PrintExpression(p->value, 3);
    printf("\n");
}

static void PrintType(ASTStatementType* t) {
    printf("\tType Statement:\n");
    printf("\t\tname: ");
    TokenPrint(&t->name);
    printf("\n");
    switch(t->type) {
        case AST_TYPE_STATEMENT_ANY:
            printf("\t\tAnyType\n");
            break;
        case AST_TYPE_STATEMENT_ENUM:
            printf("\t\tEnumType\n\t\twidth: ");
            TokenPrint(&t->as.enumType.width);
            printf("\n\t\tmembers:\n");
            for(unsigned int j = 0; j < t->as.enumType.memberCount; j++) {
                printf("\t\t\t");
                TokenPrint(&t->as.enumType.members[j]);
                printf("\n");
            }
            break;
    }
}

static void PrintBitGroup(ASTStatementBitGroup* b) {
    printf("\tBitgroup Statement:\n\t\tname: ");
    TokenPrint(&b->name);
    printf("\n\t\tparameters:\n");
    for(unsigned int j = 0; j < b->paramCount; j++) {
        printf("\t\t\tname: ");
        TokenPrint(&b->params[j].name);
        printf("\n\t\t\tvalue: ");
        TokenPrint(&b->params[j].value);
        printf("\n");
    }
    printf("\t\tsegments:\n");
    for(unsigned int j = 0; j < b->segmentCount; j++) {
        switch(b->segments[j].type) {
            case AST_BIT_GROUP_IDENTIFIER_SUBST:
                printf("\t\t\tSubstitution: ");
                break;
            case AST_BIT_GROUP_IDENTIFIER_LITERAL:
                printf("\t\t\tLiteral: ");
                break;
        }
        TokenPrint(&b->segments[j].identifier);
        printf("\n");
    }
}

void PrintAST(AST* ast) {
    printf("AST:\n");

    printf("filenames:\n");
    for(unsigned int i = 0; i < ast->fileNameCount; i++) {
        printf("\t%s\n", ast->fileNames[i]);
    }

    printf("Statements:\n");
    for(unsigned int i = 0; i < ast->statementCount; i++) {
        ASTStatement* s = &ast->statements[i];
        switch(s->type) {
            case AST_BLOCK_HEADER: PrintHeader(&s->as.header); break;
            case AST_BLOCK_OPCODE: PrintOpcode(&s->as.opcode); break;
            case AST_BLOCK_PARAMETER: PrintParameter(&s->as.parameter); break;
            case AST_BLOCK_TYPE: PrintType(&s->as.type); break;
            case AST_BLOCK_BITGROUP: PrintBitGroup(&s->as.bitGroup); break;
        }
    }

    printf("done.\n");
}