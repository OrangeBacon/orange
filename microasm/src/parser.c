#include <stdio.h>
#include "token.h"
#include "parser.h"

static void advance(Parser* parser);
static bool match(Parser* parser, TokenType type);
static bool check(Parser* parser, TokenType type);
static void syncronise(Parser* parser);
static void block(Parser* parser);
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
    advance(parser);
    while(!match(parser, TOKEN_EOF)){
        block(parser);
    }

    return !parser->hadError;
}

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

static bool match(Parser* parser, TokenType type) {
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static void syncronise(Parser* parser) {
    parser->panicMode = false;
    while(parser->current.type != TOKEN_EOF) {
        switch(parser->current.type) {
            case TOKEN_INPUT:
            case TOKEN_OPCODE:
            case TOKEN_HEADER:
            case TOKEN_OUTPUT:
            case TOKEN_MACRO:
                return;
            default:;  // do nothing
        }
        advance(parser);
    }
}

static void block(Parser* parser) {
    if(parser->panicMode) syncronise(parser);

    if(match(parser, TOKEN_OPCODE)) {
        
    } else if(match(parser, TOKEN_MACRO)) {
        
    } else if(match(parser, TOKEN_HEADER)) {
        if(parser->headerStatement){
            error(parser, "Only one header statement allowed per microcode");
        }
        parser->headerStatement = true;
    } else if(match(parser, TOKEN_INPUT)) {
        if(parser->inputStatement){
            error(parser, "Only one input statement allowed per microcode");
        }
        parser->inputStatement = true;
    } else if(match(parser, TOKEN_OUTPUT)) {
        if(parser->outputStatement){
            error(parser, "Only one output statement allowed per microcode");
        }
        parser->outputStatement = true;
    } else {
        errorAtCurrent(parser, "Expected a block type");
    }
}

static void errorAtCurrent(Parser* parser, const char* message) {
    errorAt(parser, &parser->current, message);
}

static void error(Parser* parser, const char* message) {
    errorAt(parser, &parser->previous, message);
}

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