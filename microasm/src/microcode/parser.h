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

    // has a _ statement been parsed yet?
    // allows them to only be included once per file
    // if no then is a token where the line number is -1
    // if yes then is the token representing the start of
    // the relavant block

    Token headerStatement;
    Token inputStatement;

    // the ast that is being constructed currently
    AST ast;

#ifdef debug
    bool readTests;
#endif

    DEFINE_ARRAY(bool, errorStack);
} Parser;

// initialise a new parser
void ParserInit(Parser* parser, Scanner* scan);

// run the parser
bool Parse(Parser* parse);

#ifdef debug
void expectTestStatements(Parser* parser);
#endif

void setErrorState(Parser* parser);

#endif
