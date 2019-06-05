#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "parser.h"
#include "ast.h"
#include "memory.h"

static void advance(Parser* parser);
static bool match(Parser* parser, TokenType type);
static bool check(Parser* parser, TokenType type);
static void consume(Parser* parser, TokenType type, const char* message);
static void syncronise(Parser* parser);
static void block(Parser* parser);
static void header(Parser* parser);
static Token microcodeLine(Parser* parser);
static bool blockStart(Parser* parser);
static void blockEnd(Parser* parser, bool start);
static void errorAtCurrent(Parser* parser, const char* message);
static void error(Parser* parser, const char* message);
static void errorAt(Parser* parser, Token* token, const char* message);

void ParserInit(Parser* parser, Scanner* scan) {
    parser->scanner = scan;
    parser->hadError = false;
    parser->panicMode = false;
    parser->headerStatement = false;
    parser->inputStatement = false;
    parser->outputStatement = false;
    InitMicrocode(&parser->ast);
}

bool Parse(Parser* parser) {
    // gets first token, required so not matching garbage(causes segfault)
    advance(parser);

    while(!match(parser, TOKEN_EOF)){
        // all file-level constructs are blocks of some form
        block(parser);
    }

    return !parser->hadError;
}

// reports all error tokens, returning next non error token
static void advance(Parser* parser) {
    parser->previous = parser->current;

    for(;;) {
        parser->current = ScanToken(parser->scanner);
        if(parser->current.type != TOKEN_ERROR){
            break;
        }
        errorAtCurrent(parser, parser->current.start);
    }
}

// consume a token of type type, else return false
static bool match(Parser* parser, TokenType type) {
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if(parser->current.type == type) {
        advance(parser);
        return;
    }
    errorAtCurrent(parser, message);
}

// is the next token of type type?
static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

// get to known parser state after error occured
static void syncronise(Parser* parser) {
    parser->panicMode = false;
    while(parser->current.type != TOKEN_EOF) {
        switch(parser->current.type) {
            // should mostly be able to continue parsing from these tokens
            case TOKEN_INPUT:
            case TOKEN_OPCODE:
            case TOKEN_HEADER:
            case TOKEN_OUTPUT:
            case TOKEN_MACRO:
                return;
            default:;  // do nothing - cannot calculate a known parser state
        }
        advance(parser);
    }
}

// dispatch the parser for a block level statement
static void block(Parser* parser) {
    if(match(parser, TOKEN_OPCODE)) {
        //TODO
    } else if(match(parser, TOKEN_MACRO)) {
        //TODO
    } else if(match(parser, TOKEN_HEADER)) {
        if(parser->headerStatement){
            error(parser, "Only one header statement allowed per microcode");
        }
        parser->headerStatement = true;
        header(parser);
    } else if(match(parser, TOKEN_INPUT)) {
        if(parser->inputStatement){
            error(parser, "Only one input statement allowed per microcode");
        }
        parser->inputStatement = true;
        //TODO
    } else if(match(parser, TOKEN_OUTPUT)) {
        if(parser->outputStatement){
            error(parser, "Only one output statement allowed per microcode");
        }
        parser->outputStatement = true;
        //TODO
    } else {
        errorAtCurrent(parser, "Expected a block statement");
    }

    // if error occured reset parser state to known value
    if(parser->panicMode) syncronise(parser);
}

// parses a header statement
static void header(Parser* parser) {
    bool brace = blockStart(parser);
    Token line = microcodeLine(parser);
    if(line.start != NULL) {
        errorAt(parser, &line, "Condition values not allowed in header");
    }
    blockEnd(parser, brace);
}

// parses a line of microcode commands with conditions
// returns the token of the first equals if conditions present
// else returns a token with a null start.
// return used to display error in correct location if no
// conditions allowed.
static Token microcodeLine(Parser* parser) {
    // are conditions being parsed?
    bool cond = true;

    // is this the first parse iteration?
    bool first = true;

    // the token representing the first equals token encountered
    Token equals = {.start = NULL};

    // testing code
    Line line;
    line.bits = ArenaAlloc(&parser->ast.arena, sizeof(Token*));
    line.conditions = ArenaAlloc(&parser->ast.arena, sizeof(Condition*));
    line.conditions[0].name = (Token){.type = TOKEN_COLON};
    line.conditions[0].value = (Token){.type = TOKEN_INPUT};
    line.condition1Equals = equals;
    parser->ast.head.line = line;

    for(;;) {
        consume(parser, TOKEN_IDENTIFIER, "Expected identifier");
        if(first) { // in first loop
            if(match(parser, TOKEN_EQUAL)) {
                // now know conditions are being parsed in this loop
                equals = parser->previous;
                consume(parser, TOKEN_IDENTIFIER, "Expected condition value");
            } else {
                // no conditions present, this loop parses bit names
                cond = false;
            }
        }
        if(cond && !first) {
            // condition value
            consume(parser, TOKEN_EQUAL, "Expected equals symbol");
            consume(parser, TOKEN_IDENTIFIER, "Expected condition value");
        }

        // are there more values to parse?
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
        first = false;
    }

    if(cond) {
        // seperator between conditions and bit names required
        consume(parser, TOKEN_COLON, "Expected colon");
    } else {
        // no conditions so second loop not required
        return equals;
    }

    // bits
    for(;;) {
        consume(parser, TOKEN_IDENTIFIER, "Expected bit name");
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }
    return equals;
}

// parse a single or multi-line block start and return the type parsed
// true = multi-line, false = single-line
static bool blockStart(Parser* parser) {
    if(match(parser, TOKEN_LEFT_BRACE)) {
        return true;  // multiline block
    } else if(match(parser, TOKEN_EQUAL)) {
        return false; // single line block
    }
    errorAtCurrent(parser, "Expected the start of a block");
    return false; // value does not matter
}

// expect the ending of a block based on the start type
static void blockEnd(Parser* parser, bool start) {
    if(start) {
        // optional semicolon
        match(parser, TOKEN_SEMICOLON);
        consume(parser, TOKEN_RIGHT_BRACE, "Expected a right brace at end of block");
    } else {
        consume(parser, TOKEN_SEMICOLON, "Expected a semi-colon at end of block");
    }
}

// issue error for token before advance() called
static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

// issue error for already advanced() token
static void error(Parser* parser, const char* message) {
    errorAt(parser, &parser->previous, message);
}

// print an error message at a token's position
static void errorAt(Parser* parser, Token* token, const char* message) {
    if(parser->panicMode) return;
    parser->panicMode = true;

    fprintf(stderr, "[%i:%i] Error", token->line, token->column);
    if(token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if(token->type == TOKEN_ERROR) {
        // nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}