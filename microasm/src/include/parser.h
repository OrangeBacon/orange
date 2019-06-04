#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "token.h"
#include "scanner.h"
#include "ast.h"

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

    bool headerStatement;
    bool inputStatement;
    bool outputStatement;

    // the ast that is being constructed currently
    Microcode ast;
} Parser;

// initialise a new parser
void ParserInit(Parser* parser, Scanner* scan);

// run the parser
bool Parse(Parser* parse);

#endif