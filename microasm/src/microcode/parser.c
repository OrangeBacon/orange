#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include "shared/memory.h"
#include "shared/platform.h"
#include "microcode/token.h"
#include "microcode/parser.h"
#include "microcode/ast.h"
#include "microcode/error.h"

static void advance(Parser* parser);
static bool match(Parser* parser, MicrocodeTokenType type);
static bool check(Parser* parser, MicrocodeTokenType type);
static void block(Parser* parser);
static void header(Parser* parser);
static void input(Parser* parser);
static void opcode(Parser* parser);
static Line* microcodeLine(Parser* parser);
static bool blockStart(Parser* parser);
static void blockEnd(Parser* parser, bool start);

#ifdef debug
static void errorStatement(Parser* parser);
#endif

// error messages:
// error = cannot deal with the syntax, skip until knows what is going on, fatal
// warn = semantic error, does not skip, fatal
// note = infomation output, non fatal

void ParserInit(Parser* parser, Scanner* scan, AST* ast) {
    parser->scanner = scan;
    parser->hadError = false;
    parser->panicMode = false;
    parser->headerStatement.line = -1;
    parser->inputStatement.line = -1;
#ifdef debug
    parser->readTests = false;
#endif
    ARRAY_ALLOC(bool, *parser, errorStack);
    parser->ast = ast;
}

static void newErrorState(Parser* parser) {
    PUSH_ARRAY(bool, *parser, errorStack, false);
}

static bool endErrorState(Parser* parser) {
    return POP_ARRAY(*parser, errorStack);
}

void setErrorState(Parser* parser) {
    for(unsigned int i = 0; i < parser->errorStackCount; i++) {
        parser->errorStacks[i] = true;
    }
}

bool Parse(Parser* parser) {
    // gets first token, required so not matching garbage(causes segfault)
    advance(parser);

    while(!match(parser, TOKEN_EOF)){
        // all file-level constructs are blocks of some form
        block(parser);
    }

    if(parser->inputStatement.line == -1) {
        warnAt(parser, 37, &parser->previous, "No input statement found");
    }
    if(parser->headerStatement.line == -1) {
        warnAt(parser, 38, &parser->previous, "No header statement found");
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
        errorAtCurrent(parser, 1, parser->current.data.string);
    }
}

// consume a token of type type, else return false
static bool match(Parser* parser, MicrocodeTokenType type) {
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, MicrocodeTokenType type, unsigned int code, const char* message, ...) {
    va_list args;
    va_start(args, message);
    if(parser->current.type == type) {
        advance(parser);
        va_end(args);
        return;
    }
    vErrorAtCurrent(parser, code, message, args);
    va_end(args);
}

// is the next token of type type?
static bool check(Parser* parser, MicrocodeTokenType type) {
    return parser->current.type == type;
}

// get to known parser state after error occured
static void syncronise(Parser* parser) {
    parser->panicMode = false;
    while(parser->current.type != TOKEN_EOF) {
        switch(parser->current.type) {
            // should mostly be able to continue parsing from these tokens
            case TOKEN_INPUT:
            case TOKEN_INCLUDE:
            //case TOKEN_TYPE:
            case TOKEN_OPCODE:
            case TOKEN_HEADER:
                return;
            default:;  // do nothing - cannot calculate a known parser state
        }
        advance(parser);
    }
}

// dispatch the parser for a block level statement
static void block(Parser* parser) {
    if(match(parser, TOKEN_OPCODE)) {
        opcode(parser);
    } else if(match(parser, TOKEN_HEADER)) {
        header(parser);
    } else if(match(parser, TOKEN_INPUT)) {
        input(parser);
    } else if(match(parser, TOKEN_INCLUDE)) {
        consume(parser, TOKEN_STRING, 30, "Expecting file name string");
    }
#ifdef debug
    else if(parser->readTests && match(parser, TOKEN_IDENTIFIER)
        && parser->previous.length == 1 && parser->previous.start[0] == 'E') {
        errorStatement(parser);
    }
#endif
    else {
        errorAtCurrent(parser, 6, "Expected a block statement, got %s", TokenNames[parser->current.type]);
    }

    // if error occured reset parser state to known value
    if(parser->panicMode) syncronise(parser);
}

// parses a header statement
static void header(Parser* parser) {
    bool write;
    if(parser->headerStatement.line != -1){
        bool e = warn(parser, 3, "Only one header statement allowed per microcode");
        if(e) noteAt(parser, &parser->headerStatement, "Previously declared here");
        write = false;
    } else {
        parser->headerStatement = parser->previous;
        write = true;
    }

    newErrorState(parser);

    Header head;
    ARRAY_ALLOC(BitArray, head, line);
    head.errorPoint = parser->previous;

    bool brace = blockStart(parser);

    if(brace) {
        while(!check(parser, TOKEN_EOF)) {
            if(check(parser, TOKEN_RIGHT_BRACE)) {
                break;
            }
            Line* line = microcodeLine(parser);

            if(line->hasCondition) {
                errorAt(parser, 7, &line->conditionErrorToken, "Condition values not allowed in header");
            }

            PUSH_ARRAY(BitArray, head, line, line->bitsLow);
            if(!match(parser, TOKEN_SEMICOLON)) {
                break;
            }
        }
    } else {
        Line* line = microcodeLine(parser);

        if(line->hasCondition) {
            errorAt(parser, 7, &line->conditionErrorToken, "Condition values not allowed in header");
        }

        PUSH_ARRAY(BitArray, head, line, line->bitsLow);
    }

    blockEnd(parser, brace);
    head.isValid = !endErrorState(parser);

    if(write) {
        parser->ast->head = head;
    }
}

// parses an input statement
static void input(Parser* parser) {
    bool write;
    if(parser->inputStatement.line != -1){
        bool e = warn(parser, 4, "Only one input statement allowed per microcode");
        if(e) noteAt(parser, &parser->inputStatement, "Previously declared here");
        write = false;
    } else {
        parser->inputStatement = parser->previous;
        write = true;
    }

    newErrorState(parser);

    Token inputHeadToken = parser->previous;
    bool brace = blockStart(parser);

    Input inp;
    inp.inputHeadToken = inputHeadToken;
    ARRAY_ALLOC(InputValue, inp, value);

    if(brace) {
        while(!check(parser, TOKEN_EOF)) {
            if(match(parser, TOKEN_IDENTIFIER)) {
                Token name = parser->previous;

                Token value = name;
                value.type = TOKEN_NUMBER;
                value.data.value = 1;

                if(match(parser, TOKEN_COLON)) {
                    consume(parser, TOKEN_NUMBER, 8, "Expected input flag width, got %s", TokenNames[parser->current.type]);
                    value = parser->previous;
                }
                PUSH_ARRAY(InputValue, inp, value, ((InputValue){.name = name, .value = value}));
                if(!match(parser, TOKEN_SEMICOLON)){
                    break;
                }
                while(match(parser, TOKEN_SEMICOLON)){}
            } else {
                break;
            }
        }
    } else {
        consume(parser, TOKEN_IDENTIFIER, 11, "Expected input name");
        Token name = parser->previous;

        Token value = name;
        value.type = TOKEN_NUMBER;
        value.data.value = 1;

        if(match(parser, TOKEN_COLON)) {
            consume(parser, TOKEN_NUMBER, 9, "Expected input flag width, got %s", TokenNames[parser->current.type]);
            value = parser->previous;
        }
        PUSH_ARRAY(InputValue, inp, value, ((InputValue){.name = name, .value = value}));
    }
    if(write) {
        parser->ast->inp = inp;
    }
    blockEnd(parser, brace);
    parser->ast->inp.isValid = !endErrorState(parser);
}

static void opcode(Parser* parser) {
    newErrorState(parser);
    
    OpCode code;
    ARRAY_ALLOC(Line*, code, line);

    consume(parser, TOKEN_NUMBER, 30, "Expected opcode number, got %s", TokenNames[parser->current.type]);
    code.id = parser->previous;
    consume(parser, TOKEN_IDENTIFIER, 26, "Expected opcode name, got %s", TokenNames[parser->current.type]);
    code.name = parser->previous;

    consume(parser, TOKEN_LEFT_PAREN, 27, "Expected left paren, got %s", TokenNames[parser->current.type]);

    while(match(parser, TOKEN_IDENTIFIER)) {
        match(parser, TOKEN_IDENTIFIER);
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }

    consume(parser, TOKEN_RIGHT_PAREN, 28, "Expected right paren, got %s", TokenNames[parser->current.type]);
    
    bool brace = blockStart(parser);

    if(brace) {
        while(!check(parser, TOKEN_EOF)) {
            if(check(parser, TOKEN_RIGHT_BRACE)) {
                break;
            }
            Line* line = microcodeLine(parser);
            PUSH_ARRAY(Line, code, line, line);
            if(!match(parser, TOKEN_SEMICOLON)) {
                break;
            }
        }
    } else {
        Line* line = microcodeLine(parser);
        PUSH_ARRAY(Line, code, line, line);
    }

    blockEnd(parser, brace);

    code.isValid = !endErrorState(parser);
    PUSH_ARRAY(OpCode, *parser->ast, opcode, code);
}

#ifdef debug
static void errorStatement(Parser* parser) {
    Error error;
    consume(parser, TOKEN_NUMBER, 33, "Expected error id, got %s", TokenNames[parser->current.type]);
    error.id = parser->previous.data.value;
    consume(parser, TOKEN_NUMBER, 34, "Expected error line, got %s", TokenNames[parser->current.type]);
    error.token.line = parser->previous.data.value;
    consume(parser, TOKEN_COLON, 31, "Expected colon");
    consume(parser, TOKEN_NUMBER, 35, "Expected error column, got %s", TokenNames[parser->current.type]);
    error.token.column = parser->previous.data.value;
    consume(parser, TOKEN_SEMICOLON, 32, "Expected semicolon");

    PUSH_ARRAY(Error, *parser->ast, expectedError, error);
}

void expectTestStatements(Parser* parser) {
    parser->readTests = true;
}

#endif

static BitArray parseMicrocodeBitArray(Parser* parser) {
    BitArray result;
    ARRAY_ALLOC(Token, result, data);
    while(match(parser, TOKEN_IDENTIFIER)) {
        PUSH_ARRAY(Token, result, data, parser->previous);
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }
    return result;
}

// parses a line of microcode commands with conditions
// returns the line ast representing what was parsed
static Line* microcodeLine(Parser* parser) {
    Line* line = ArenaAlloc(sizeof(Line));
    line->conditionErrorToken = (Token){.type = TOKEN_NULL};

    if(check(parser, TOKEN_NUMBER)) {
        line->hasCondition = true;
        advance(parser);
        bool swap;
        if(parser->previous.data.value == 1) {
            swap = false;
        } else if(parser->previous.data.value == 0) {
            swap = true;
        } else {
            warnAt(parser, 49, &parser->previous, "Condition values can only be 0 or 1");
        }
        consume(parser, TOKEN_COLON, 50, "Expected colon after condition");
        line->bitsHigh = parseMicrocodeBitArray(parser);
        consume(parser, TOKEN_SEMICOLON, 51, "Semicolon expected between parts "
            "of conditional microcode line");
        consume(parser, TOKEN_NUMBER, 52, "Expected second condition value");
        if(parser->previous.data.value == 1) {
            if(!swap) {
                warnAt(parser, 53, &parser->previous, "Condition value 1 repeated");
            }
        } else if(parser->previous.data.value == 0) {
            if(swap) {
                warnAt(parser, 54, &parser->previous, "Condition value 0 repeated");
            }
        } else {
            warnAt(parser, 55, &parser->previous, "Condition values can only be 0 or 1");
        }
        consume(parser, TOKEN_COLON, 56, "Expected colon after condition");
        line->bitsLow = parseMicrocodeBitArray(parser);

        if(swap) {
            BitArray temp = line->bitsHigh;
            line->bitsHigh = line->bitsLow;
            line->bitsLow = temp;
        }
    } else {
        line->hasCondition = false;
        line->bitsHigh = line->bitsLow = parseMicrocodeBitArray(parser);
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
    errorAtCurrent(parser, 10, "Expected the start of a block");
    return false; // value does not matter
}

// expect the ending of a block based on the start type
static void blockEnd(Parser* parser, bool start) {
    if(start) {
        // optional semicolon
        match(parser, TOKEN_SEMICOLON);
        consume(parser, TOKEN_RIGHT_BRACE, 17, "Expected a right brace at end of block");
    } else {
        consume(parser, TOKEN_SEMICOLON, 18, "Expected a semi-colon at end of block");
    }
}
