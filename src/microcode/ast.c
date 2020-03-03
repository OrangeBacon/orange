#include "microcode/token.h"
#include "microcode/ast.h"
#include "shared/memory.h"
#include "shared/log.h"
#include "microcode/parser.h"

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