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

static Error errUnexpectedCharacter = {0};
static void advanceErrors() {
    newErrCurrent(&errUnexpectedCharacter, ERROR_SYNTAX, "%s");
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
        error(parser, &errUnexpectedCharacter, parser->current.data.string);
    }
}

// consume a token of type type, else return false
static bool match(Parser* parser, MicrocodeTokenType type) {
    CONTEXT(DEBUG, "Parser token match");
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, Error* error, ...) {
    CONTEXT(DEBUG, "consume valid token");
    va_list args;
    va_start(args, error);
    if(parser->current.type == error->consumeType) {
        advance(parser);
        va_end(args);
        DEBUG("Found %s token", TokenNames[error->consumeType]);
        return;
    }
    INFO("Could not find %s token", TokenNames[error->consumeType]);
    vError(parser, error, args);
    va_end(args);
}

// get to known parser state after error occured
static void syncronise(Parser* parser) {
    CONTEXT(DEBUG, "Error syncronisation");
    parser->panicMode = false;
    while(parser->current.type != TOKEN_EOF) {
        switch(parser->current.type) {
            // should mostly be able to continue parsing from these tokens
            case TOKEN_INCLUDE:
            case TOKEN_OPSIZE:
            case TOKEN_PHASE:
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

static Error errConditionValue = {0};
static Error errConditionValueRepeated = {0};
static Error errConditionColon = {0};
static Error errConditionSemicolon = {0};
static Error errNoSecondConditionLine = {0};
static void microcodeLineErrors() {
    newErrPrevious(&errConditionValue, ERROR_SEMANTIC, "Condition values can only be 0 or 1");
    newErrPrevious(&errConditionValueRepeated, ERROR_SEMANTIC, "Condition value %u repeated");
    newErrConsume(&errConditionColon, ERROR_SYNTAX,
        TOKEN_COLON, "Expected colon after condition");
    newErrConsume(&errConditionSemicolon, ERROR_SYNTAX,
        TOKEN_SEMICOLON, "Semicolon expected between parts of conditional microcode line");
    newErrConsume(&errNoSecondConditionLine, ERROR_SYNTAX,
        TOKEN_NUMBER, "Expected second condition value");
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
            error(parser, &errConditionValue);
        }
        consume(parser, &errConditionColon);
        line->bitsHigh = parseMicrocodeBitArray(parser);
        consume(parser, &errConditionSemicolon);
        consume(parser, &errNoSecondConditionLine);
        if(parser->previous.data.value == 1) {
            if(!swap) {
                error(parser, &errConditionValueRepeated, 1);
            }
        } else if(parser->previous.data.value == 0) {
            if(swap) {
                error(parser, &errConditionValueRepeated, 0);
            }
        } else {
            error(parser, &errConditionValue);
        }
        consume(parser, &errConditionColon);
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

static Error errParameterColon;
static Error errParameterNumber;
static void parameterErrors() {
    newErrConsume(&errParameterColon, ERROR_SYNTAX,
        TOKEN_COLON, "Missing colon seperating %s from its value");
    newErrConsume(&errParameterNumber, ERROR_SYNTAX,
        TOKEN_NUMBER, "Missing value of %s parameter");
}

static void parameter(Parser* parser, const char* type) {
    consume(parser, &errParameterColon, type);
    consume(parser, &errParameterNumber, type);

    // works for now
    if(type[0] == 'o') {
        parser->ast->opsize = parser->previous;
    } else {
        parser->ast->phase = parser->previous;
    }
}

static Error errMultipleHeaders = {0};
static Error errHeaderCondition = {0};
static Error errBlockStart = {0};
static Error errBlockEnd = {0};
static void headerErrors() {
    newErrPrevious(&errMultipleHeaders, ERROR_SEMANTIC,
        "Only one header statement allowed per microcode");
    newErrNoteAt(&errMultipleHeaders, "Previously declared here");
    newErrAt(&errHeaderCondition, ERROR_SEMANTIC, "Condition values not allowed in header");
    newErrConsume(&errBlockStart, ERROR_SYNTAX,
        TOKEN_LEFT_BRACE, "Expected \"{\" at start of block");
    newErrConsume(&errBlockEnd, ERROR_SYNTAX,
        TOKEN_RIGHT_BRACE, "Expected \"}\" at end of block");
}

// parses a header statement
static void header(Parser* parser) {
    CONTEXT(INFO, "Parsing header statement");

    bool write;
    if(parser->headerStatement.line != -1){
        error(parser, &errMultipleHeaders, &parser->headerStatement);
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

    consume(parser, &errBlockStart);

    while(!check(parser, TOKEN_EOF)) {
        if(check(parser, TOKEN_RIGHT_BRACE)) {
            break;
        }
        Line* line = microcodeLine(parser);

        if(line->hasCondition) {
            INFO("Found condition in header statement");
            error(parser, &errHeaderCondition, &line->conditionErrorToken);
        }

        PUSH_ARRAY(BitArray, head, line, line->bitsLow);
        if(!match(parser, TOKEN_SEMICOLON)) {
            break;
        }
    }

    consume(parser, &errBlockEnd);

    if(write) {
        parser->ast->head = head;
    }

    parser->ast->head.isValid = !endErrorState(parser);
    parser->ast->head.isPresent = true;
    INFO("Header statement parsed");
}

static Error errOpcodeID = {0};
static Error errOpcodeName = {0};
static Error errOpcodeParamStart = {0};
static Error errOpcodeParamEnd = {0};
static void opcodeErrors() {
    newErrConsume(&errOpcodeID, ERROR_SYNTAX,
        TOKEN_NUMBER, "Expected opcode number, got %s");
    newErrConsume(&errOpcodeName, ERROR_SYNTAX,
        TOKEN_IDENTIFIER, "Expected opcode name, got %s");
    newErrConsume(&errOpcodeParamStart, ERROR_SYNTAX,
        TOKEN_LEFT_PAREN, "Expected left paren, got %s");
    newErrConsume(&errOpcodeParamEnd, ERROR_SYNTAX,
        TOKEN_RIGHT_PAREN, "Expected right paren, got %s");
}

static void opcode(Parser* parser) {
    CONTEXT(INFO, "Parsing opcode statement");
    newErrorState(parser);

    OpCode code;
    ARRAY_ALLOC(Line*, code, line);

    consume(parser, &errOpcodeID, TokenNames[parser->current.type]);
    code.id = parser->previous;
    consume(parser, &errOpcodeName, TokenNames[parser->current.type]);
    code.name = parser->previous;

    consume(parser, &errOpcodeParamStart, TokenNames[parser->current.type]);

    while(match(parser, TOKEN_IDENTIFIER)) {
        match(parser, TOKEN_IDENTIFIER);
        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }

    consume(parser, &errOpcodeParamEnd, TokenNames[parser->current.type]);

    INFO("Parsed opcode statement header");

    consume(parser, &errBlockStart);

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

    consume(parser, &errBlockEnd);

    code.isValid = !endErrorState(parser);
    PUSH_ARRAY(OpCode, *parser->ast, opcode, code);
    INFO("Parsed opcode statement");
}

static Error errCouldNotFindFile = {0};
static Error errMissingIncludeFileName = {0};
static void includeErrors() {
    newErrPrevious(&errCouldNotFindFile, ERROR_SEMANTIC, "Could not find file \"%s\"");
    newErrCurrent(&errMissingIncludeFileName, ERROR_SYNTAX,
        "Expected string containing file name to include.");
}

static void include(Parser* parser) {
    CONTEXT(INFO, "Parsing include statement");
    if(match(parser, TOKEN_STRING)) {
        DEBUG("Searching for file in include paths");

        // TODO: change from list of files to include tree
        char* foundFileName;
        FILE* file = pathStackSearchFileList(
            parser->previous.data.string,
            "uasm",
            parser->ast->fileNameCount,
            parser->ast->fileNames,
            &foundFileName);
        if(file == NULL) {
            error(parser, &errCouldNotFindFile, parser->previous.data.string);
            return;
        }

        INFO("Found file to include, parsing");
        Scanner newScanner;
        Parser newParser;
        ScannerInit(&newScanner, readFilePtr(file), foundFileName);
        Parse(&newParser, &newScanner, parser->ast);

        if(newParser.hadError){
            parser->hadError = true;
            for(unsigned int i = 0; i < newParser.errorCount; i++) {
                PUSH_ARRAY(EmittedError, *parser, error, newParser.errors[i]);
            }
        }

        INFO("Include parse completed");
    } else {
        INFO("Could not find input file string in source");
        error(parser, &errMissingIncludeFileName);
    }
}

static Error errExpectedBlock = {0};
static void blockErrors() {
    newErrPrevious(&errExpectedBlock, ERROR_SYNTAX, "Expected a block statement, got %s");
}

// dispatch the parser for a block level statement
static void block(Parser* parser) {
    CONTEXT(INFO, "Parsing block statement");
    advance(parser);

    switch(parser->previous.type) {
        case TOKEN_OPCODE: opcode(parser); break;
        case TOKEN_HEADER: header(parser); break;
        case TOKEN_INCLUDE: include(parser); break;
        case TOKEN_OPSIZE: parameter(parser, "opsize"); break;
        case TOKEN_PHASE: parameter(parser, "phase"); break;
        default:
            INFO("Could not find valid block statement");
            error(parser, &errExpectedBlock, TokenNames[parser->previous.type]);
    }

    // if error occured reset parser state to known value
    if(parser->panicMode) syncronise(parser);
}

static Error errRecursiveInclude = {0};
static void runParserErrors() {
    newErrCurrent(&errRecursiveInclude, ERROR_SEMANTIC,
        "Recursive include detected of file \"%s\"");
}

static bool runParser(Parser* parser) {
    CONTEXT(INFO, "Running microcode parser");

    // gets first token, required so not matching garbage(causes segfault)
    advance(parser);

    DEBUG("Checking filename is valid");
    for(unsigned int i = 0; i < parser->ast->fileNameCount; i++) {
        if(parser->ast->fileNames[i] == parser->scanner->fileName) {
            error(parser, &errRecursiveInclude, parser->scanner->fileName);
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

static bool errorsInitialised;
typedef void (*errorInitialiser)();
static errorInitialiser errorInitialisers[] = {
    advanceErrors,
    microcodeLineErrors,
    parameterErrors,
    headerErrors,
    opcodeErrors,
    includeErrors,
    blockErrors,
    runParserErrors
};

void InitParser() {
    CONTEXT(INFO, "Initialising parser data");
   
    if(!errorsInitialised) {
        for(unsigned int i = 0; i < sizeof(errorInitialisers)/sizeof(errorInitialiser); i++) {
            errorInitialisers[i]();
        }
    }
}

bool Parse(Parser* parser, Scanner* scan, AST* ast) {
    CONTEXT(INFO, "Initialising new parser");

    if(!errorsInitialised)InitParser();

    parser->scanner = scan;
    parser->hadError = false;
    parser->panicMode = false;
    parser->headerStatement.line = -1;
    parser->inputStatement.line = -1;
    ARRAY_ALLOC(bool, *parser, errorStack);
    ARRAY_ALLOC(EmittedError, *parser, error);
    parser->ast = ast;

    return runParser(parser);
}