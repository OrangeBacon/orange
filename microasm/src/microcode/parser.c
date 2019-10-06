#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "shared/platform.h"
#include "shared/path.h"
#include "shared/log.h"
#include "microcode/token.h"
#include "microcode/parser.h"
#include "microcode/ast.h"
#include "microcode/error.h"

// error messages:
// error = cannot deal with the syntax, skip until knows what is going on, fatal
// warn = semantic error, does not skip, fatal
// note = infomation output, non fatal

static void newErrorState(Parser* parser) {
    CONTEXT(DEBUG, "New error state");
    PUSH_ARRAY(bool, *parser, errorStack, false);
}

static bool endErrorState(Parser* parser) {
    CONTEXT(DEBUG, "Exited error state");
    return POP_ARRAY(*parser, errorStack);
}

void setErrorState(Parser* parser) {
    CONTEXT(DEBUG, "Setting error state to error");
    for(unsigned int i = 0; i < parser->errorStackCount; i++) {
        parser->errorStacks[i] = true;
    }
    INFO("All error stacks set to error");
}

// is the next token of type type?
static bool check(Parser* parser, MicrocodeTokenType type) {
    return parser->current.type == type;
}

// reports all error tokens, returning next non error token
static void advance(Parser* parser) {
    CONTEXT(DEBUG, "Get next valid token");
    parser->previous = parser->current;

    for(;;) {
        parser->current = ScanToken(parser->scanner);
        if(!check(parser, TOKEN_ERROR)){
            TRACE("Found valid token");
            break;
        }
        DEBUG("Found error token");
        errorAtCurrent(parser, 1, parser->current.data.string);
    }
}

// consume a token of type type, else return false
static bool match(Parser* parser, MicrocodeTokenType type) {
    CONTEXT(DEBUG, "Parser token match");
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, MicrocodeTokenType type, unsigned int code, const char* message, ...) {
    CONTEXT(DEBUG, "consume valid token");
    va_list args;
    va_start(args, message);
    if(parser->current.type == type) {
        advance(parser);
        va_end(args);
        DEBUG("Found %s token", TokenNames[type]);
        return;
    }
    INFO("Could not find %s token", TokenNames[type]);
    vErrorAtCurrent(parser, code, message, args);
    va_end(args);
}

// get to known parser state after error occured
static void syncronise(Parser* parser) {
    CONTEXT(DEBUG, "Error syncronisation");
    parser->panicMode = false;
    while(parser->current.type != TOKEN_EOF) {
        switch(parser->current.type) {
            // should mostly be able to continue parsing from these tokens
            case TOKEN_INPUT:
            case TOKEN_INCLUDE:
            //case TOKEN_TYPE:
            case TOKEN_OPCODE:
            case TOKEN_HEADER:
                INFO("Found valid parser state");
                return;
            default:;  // do nothing - cannot calculate a known parser state
        }
        advance(parser);
    }
    INFO("Could not detect valid parser state");
}

// parse a single or multi-line block start and return the type parsed
// true = multi-line, false = single-line
static bool blockStart(Parser* parser) {
    CONTEXT(INFO, "Parsing block statement");
    if(match(parser, TOKEN_LEFT_BRACE)) {
        DEBUG("Detected multi-line block");
        return true;  // multiline block
    } else if(match(parser, TOKEN_EQUAL)) {
        DEBUG("Detected single-statement block");
        return false; // single line block
    }
    INFO("Could not detect block statement");
    errorAtCurrent(parser, 10, "Expected the start of a block");
    return false; // value does not matter
}

// expect the ending of a block based on the start type
static void blockEnd(Parser* parser, bool start) {
    CONTEXT(INFO, "Finishing block statement");
    if(start) {
        // optional semicolon
        match(parser, TOKEN_SEMICOLON);
        consume(parser, TOKEN_RIGHT_BRACE, 17, "Expected a right brace at end of block");
    } else {
        consume(parser, TOKEN_SEMICOLON, 18, "Expected a semi-colon at end of block");
    }
}

static BitArray parseMicrocodeBitArray(Parser* parser) {
    CONTEXT(INFO, "Parsing microcode bit array");
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
    CONTEXT(INFO, "Parsing single microcode line");
    Line* line = ArenaAlloc(sizeof(Line));
    line->conditionErrorToken = (Token){.type = TOKEN_NULL};

    if(check(parser, TOKEN_NUMBER)) {
        INFO("Microcode line has a condition");
        line->hasCondition = true;
        advance(parser);
        bool swap = false;
        if(parser->previous.data.value == 1) {
            swap = false;
        } else if(parser->previous.data.value == 0) {
            INFO("Microcode line has swapped condition values");
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
        INFO("No condition detected");
        line->hasCondition = false;
        line->bitsHigh = line->bitsLow = parseMicrocodeBitArray(parser);
    }

    return line;
}

// parses a header statement
static void header(Parser* parser) {
    CONTEXT(INFO, "Parsing header statement");
    INFO("Parsing header statement");
    bool write;
    if(parser->headerStatement.line != -1){
        bool e = warn(parser, 3, "Only one header statement allowed per microcode");
        if(e) noteAt(parser, &parser->headerStatement, "Previously declared here");
        write = false;
        INFO("Header statement duplication error");
    } else {
        parser->headerStatement = parser->previous;
        write = true;
    }

    newErrorState(parser);

    Header head;
    ARRAY_ALLOC(BitArray, head, line);
    head.errorPoint = parser->previous;

    bool brace = blockStart(parser);

    while(!check(parser, TOKEN_EOF)) {
        if(check(parser, TOKEN_RIGHT_BRACE)) {
            break;
        }
        Line* line = microcodeLine(parser);

        if(line->hasCondition) {
            INFO("Found condition in header statement");
            errorAt(parser, 7, &line->conditionErrorToken, "Condition values not allowed in header");
        }

        PUSH_ARRAY(BitArray, head, line, line->bitsLow);
        if(!match(parser, TOKEN_SEMICOLON) ||!brace) {
            break;
        }
    }

    blockEnd(parser, brace);

    if(write) {
        parser->ast->head = head;
    }

    parser->ast->head.isValid = !endErrorState(parser);
    parser->ast->head.isPresent = true;
    INFO("Header statement parsed");
}

// parses an input statement
static void input(Parser* parser) {
    CONTEXT(INFO, "Parsing input statement");
    INFO("Parsing input statment");

    bool write;
    if(parser->inputStatement.line != -1){
        bool e = warn(parser, 4, "Only one input statement allowed per microcode");
        if(e) noteAt(parser, &parser->inputStatement, "Previously declared here");
        write = false;
        INFO("Input statement duplication error");
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
        INFO("Parsing multiple statement input statement");
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
                DEBUG("Pushed input value struct to input statement");
                if(!match(parser, TOKEN_SEMICOLON)){
                    break;
                }
                while(match(parser, TOKEN_SEMICOLON)){}
            } else {
                break;
            }
        }
    } else {
        INFO("Parsing single identifier input statement");
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
        DEBUG("Pushed input value struct to input statement");
    }

    blockEnd(parser, brace);

    if(write) {
        parser->ast->inp = inp;
    }

    parser->ast->inp.isValid = !endErrorState(parser);
    parser->ast->inp.isPresent = true;
    INFO("Parsed input statment");
}

static void opcode(Parser* parser) {
    CONTEXT(INFO, "Parsing opcode statement");
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

    INFO("Parsed opcode statement header");

    bool brace = blockStart(parser);

    if(brace) {
        DEBUG("Parsing long opcode statement");
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
        DEBUG("Parsing short opcode statement");
        Line* line = microcodeLine(parser);
        PUSH_ARRAY(Line, code, line, line);
    }

    blockEnd(parser, brace);

    code.isValid = !endErrorState(parser);
    PUSH_ARRAY(OpCode, *parser->ast, opcode, code);
    INFO("Parsed opcode statement");
}

static void include(Parser* parser) {
    CONTEXT(INFO, "Parsing include statement");
    if(match(parser, TOKEN_STRING)) {
        DEBUG("Searching for file in include paths");
        char* fileName = (char*)parser->previous.data.string;
        const char* ext = pathGetExtension(fileName);
        if(ext == NULL || strcmp(ext, "uasm") != 0) {
            char* tempBuffer = ArenaAlloc(sizeof(char) * (strlen(fileName) + 6));
            strcpy(tempBuffer, fileName);
            strcat(tempBuffer, ".uasm");
            fileName = tempBuffer;
        }

        PathStack searchList;
        pathStackInit(&searchList);

        // TODO: change from list of files to include tree
        // TODO: move path logic to path.c
        pathStackAddFolderSection(&searchList, ".");
        for(unsigned int i = 0; i < parser->ast->fileNameCount; i++) {
            pathStackAddFolderSection(&searchList, parser->ast->fileNames[i]);
        }

        char* foundFileName;
        FILE* file = pathStackSearchFile(&searchList, fileName, &foundFileName);
        if(file == NULL) {
            errorAt(parser, 700, &parser->previous, "Could not find file \"%s\"", fileName);
            return;
        }

        INFO("Found file to include, parsing");
        Scanner newScanner;
        Parser newParser;
        ScannerInit(&newScanner, readFilePtr(file), foundFileName);
        ParserInit(&newParser, &newScanner, parser->ast);
        Parse(&newParser);
        INFO("Include parse completed");
    } else {
        INFO("Could not find input file string in source");
        errorAt(parser, 401, &parser->current, "Expected string containing file name to include.");
    }
}

// dispatch the parser for a block level statement
static void block(Parser* parser) {
    CONTEXT(INFO, "Parsing block statement");
    if(match(parser, TOKEN_OPCODE)) {
        opcode(parser);
    } else if(match(parser, TOKEN_HEADER)) {
        header(parser);
    } else if(match(parser, TOKEN_INPUT)) {
        input(parser);
    } else if(match(parser, TOKEN_INCLUDE)) {
        include(parser);
    } else {
        INFO("Could not find valid block statement");
        errorAtCurrent(parser, 6, "Expected a block statement, got %s", TokenNames[parser->current.type]);
    }

    // if error occured reset parser state to known value
    if(parser->panicMode) syncronise(parser);
}

bool Parse(Parser* parser) {
    CONTEXT(INFO, "Starting new microcode parser");

    // gets first token, required so not matching garbage(causes segfault)
    advance(parser);

    DEBUG("Checking filename is valid");
    for(unsigned int i = 0; i < parser->ast->fileNameCount; i++) {
        if(parser->ast->fileNames[i] == parser->scanner->fileName) {
            errorAt(parser, 400, &parser->current, "Recursive include detected of file \"%s\"",
                parser->scanner->fileName);
            return false;
        }
    }
    PUSH_ARRAY(const char*, *parser->ast, fileName, parser->scanner->fileName);
    DEBUG("Filename checks passed");

    INFO("Starting parsing");
    while(!match(parser, TOKEN_EOF)){
        // all file-level constructs are blocks of some form
        block(parser);
    }

    INFO("Reached end of input file");
    return !parser->hadError;
}

void ParserInit(Parser* parser, Scanner* scan, AST* ast) {
    CONTEXT(INFO, "Initialising microcode parser");
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