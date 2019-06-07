#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "token.h"
#include "parser.h"
#include "ast.h"
#include "memory.h"
#include "platform.h"
#include "error.h"

static void advance(Parser* parser);
static bool match(Parser* parser, OrangeTokenType type);
static bool check(Parser* parser, OrangeTokenType type);
static void block(Parser* parser);
static void header(Parser* parser, bool write);
static void input(Parser* parser, bool write);
static Line* microcodeLine(Parser* parser);
static bool blockStart(Parser* parser);
static void blockEnd(Parser* parser, bool start);

// error messages:
// error = cannot deal with the syntax, skip until knows what is going on, fatal
// warn = semantic error, does not skip, fatal
// note = infomation output, non fatal

void ParserInit(Parser* parser, Scanner* scan) {
    parser->scanner = scan;
    parser->hadError = false;
    parser->panicMode = false;
    parser->headerStatement.line = -1;
    parser->inputStatement.line = -1;
    parser->outputStatement.line = -1;
    InitMicrocode(&parser->ast, scan->fileName);
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
static bool match(Parser* parser, OrangeTokenType type) {
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, OrangeTokenType type, const char* message, ...) {
    va_list args;
    va_start(args, message);
    if(parser->current.type == type) {
        advance(parser);
        return;
    }
    vErrorAtCurrent(parser, message, args);
}

// is the next token of type type?
static bool check(Parser* parser, OrangeTokenType type) {
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
        if(parser->headerStatement.line != -1){
            bool e = warn(parser, "Only one header statement allowed per microcode");
            if(e) noteAt(parser, &parser->headerStatement, "Previously declared here");
            header(parser, false);
        } else {
            parser->headerStatement = parser->previous;
            header(parser, true);
        }
    } else if(match(parser, TOKEN_INPUT)) {
        if(parser->inputStatement.line != -1){
            bool e = warn(parser, "Only one input statement allowed per microcode");
            if(e) noteAt(parser, &parser->inputStatement, "Previously declared here");
            input(parser, false);
        } else {
            parser->inputStatement = parser->previous;
            input(parser, true);
        }
    } else if(match(parser, TOKEN_OUTPUT)) {
        if(parser->outputStatement.line != -1){
            bool e = warn(parser, "Only one output statement allowed per microcode");
            if(e) noteAt(parser, &parser->outputStatement, "Previously declared here");
        } else {
            parser->outputStatement = parser->previous;
        }
        //TODO
    } else {
        errorAtCurrent(parser, "Expected a block statement, got %s", TokenNames[parser->current.type]);
    }

    // if error occured reset parser state to known value
    if(parser->panicMode) syncronise(parser);
}

// parses a header statement
static void header(Parser* parser, bool write) {
    bool brace = blockStart(parser);
    Line* line = microcodeLine(parser);
    if(line->conditionCount != 0) {
        errorAt(parser, &line->condition1Equals, "Condition values not allowed in header");
    }
    if(write) {
        COPY_ARRAY(*line, parser->ast.head, bit);
    }
    blockEnd(parser, brace);
}

// reads, parses and returns an unsigned integer
static unsigned int readUInt(Parser* parser, int defaultVal) {
    consume(parser, TOKEN_IDENTIFIER, "Expected input value, got %s", TokenNames[parser->current.type]);
    char* endPtr;
    long val = strtol(parser->previous.start, &endPtr, 10);
    if(endPtr != parser->previous.start + parser->previous.length) {
        warn(parser, "Could not parse token as number");
        return defaultVal;
    } else if (val < 1 || val > INT_MAX) {
        warn(parser, "Input values must be between 1 and INT_MAX");
        return defaultVal;
    }
    return (unsigned int)val;
}

// parses an input statement
static void input(Parser* parser, bool write) {
    bool brace = blockStart(parser);

    Input inp;
    ARRAY_ALLOC(InputValue, inp, value);

    if(brace) {
        while(true) {
            if(match(parser, TOKEN_IDENTIFIER)) {
                Token name = parser->previous;
                unsigned int value = 1;
                if(match(parser, TOKEN_COLON)) {
                    value = readUInt(parser, value);
                }
                PUSH_ARRAY(InputValue, inp, value, ((InputValue){.name = name, .value = value}));
                if(!match(parser, TOKEN_SEMICOLON)){
                    break;
                }
            } else {
                break;
            }
        }
    } else {
        consume(parser, TOKEN_IDENTIFIER, "Expected input name");
        Token name = parser->previous;
        unsigned int value = 1;
        if(match(parser, TOKEN_COLON)) {
            value = readUInt(parser, value);
        }
        PUSH_ARRAY(InputValue, inp, value, ((InputValue){.name = name, .value = value}));
    }
    if(write) {
        parser->ast.inp = inp;
    }
    blockEnd(parser, brace);
}

// parses a line of microcode commands with conditions
// returns the line ast representing what was parsed
static Line* microcodeLine(Parser* parser) {
    // are conditions being parsed?
    bool cond = true;

    // is this the first parse iteration?
    bool first = true;

    Line* line = ArenaAlloc(sizeof(Line));
    ARRAY_ALLOC(Token, *line, bit);
    ARRAY_ALLOC(Condition, *line, condition);
    line->condition1Equals = (Token){.start = NULL};

    for(;;) {
        consume(parser, TOKEN_IDENTIFIER, "Expected identifier");
        Token name = parser->previous;
        if(first) { // in first loop iteration
            cond = check(parser, TOKEN_EQUAL);
        }
        if(cond) {
            consume(parser, TOKEN_EQUAL, "Expected equals symbol");

            if(first) {
                line->condition1Equals = parser->previous;
            }

            consume(parser, TOKEN_IDENTIFIER, "Expected condition value");

            Condition condition;
            condition.name = name;
            condition.value = parser->previous;
            PUSH_ARRAY(Condition, *line, condition, condition);
        } else {
            PUSH_ARRAY(Token, *line, bit, name);
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
        return line;
    }

    // bits
    for(;;) {
        consume(parser, TOKEN_IDENTIFIER, "Expected bit name");
        PUSH_ARRAY(Token, *line, bit, parser->previous);
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }

    return line;
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