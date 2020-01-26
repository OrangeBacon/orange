#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "shared/memory.h"
#include "microcode/token.h"
#include "microcode/scanner.h"
#include "microcode/ast.h"

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

    DEFINE_ARRAY(bool, errorStack);

    DEFINE_ARRAY(struct Error*, error);
} Parser;

// initialise a new parser
void InitParser();

// run the parser
bool Parse(Parser* parse, Scanner* scan, AST* ast);

void setErrorState(Parser* parser);

#endif
