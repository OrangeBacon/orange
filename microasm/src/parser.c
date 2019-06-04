#include <stdio.h>
#include "token.h"
#include "parser.h"

static void advance(Parser* parser);
static bool match(Parser* parser, TokenType type);
static bool check(Parser* parser, TokenType type);
static void consume(Parser* parser, TokenType type, const char* message);
static void syncronise(Parser* parser);
static void block(Parser* parser);
static void header(Parser* parser);
static void microcodeLine(Parser* parser);
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

static void header(Parser* parser) {
    bool x = blockStart(parser);
    microcodeLine(parser);
    blockEnd(parser, x);
}

static void microcodeLine(Parser* parser) {
    // conditions
    // consume(parser, TOKEN_IDENTIFIER, "Expected identifier");

    // bits
    for(;;) {
        consume(parser, TOKEN_IDENTIFIER, "Expected bit name");
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }
}

static bool blockStart(Parser* parser) {
    if(match(parser, TOKEN_LEFT_BRACE)) {
        return true;  // multiline block
    } else if(match(parser, TOKEN_EQUAL)) {
        return false; // single line block
    }
    errorAtCurrent(parser, "Expected the start of a block");
    return false; // value does not matter
}

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