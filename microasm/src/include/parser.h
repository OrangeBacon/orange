#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "scanner.h"

typedef struct Parser {
    Scanner* scanner;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    bool headerStatement;
    bool inputStatement;
    bool outputStatement;
} Parser;

// initialise a new parser
void ParserInit(Parser* parser, Scanner* scan);

// run the parser
bool Parse(Parser* parse);

#endif