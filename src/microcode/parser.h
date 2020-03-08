#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "shared/memory.h"
#include "microcode/token.h"
#include "microcode/scanner.h"
#include "microcode/ast.h"

typedef enum Precidence {
    PREC_NONE,
    PREC_COMMA,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precidence;

// state required to create the syntax tree
typedef struct Parser {
    // token list
    Scanner* scanner;

    // what is being parsed now
    Token current;

    // what was parsed (only used for errors)
    Token previous;

    // has an error occured at any point so far
    bool hadError;

    // should the compiler be recovering from an error currently
    bool panicMode;

    // the ast that is being constructed currently
    AST* ast;

    ARRAY_DEFINE(bool, errorStack);

    ARRAY_DEFINE(struct Error*, error);
} Parser;

// run the parser
bool Parse(Parser* parse, Scanner* scan, AST* ast);

void setErrorState(Parser* parser);

#endif
