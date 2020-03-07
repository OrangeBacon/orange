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

static void newErrorState(Parser* parser) {
    CONTEXT(DEBUG, "New error state");
    ARRAY_PUSH(*parser, errorStack, (bool)false);
}

static bool endErrorState(Parser* parser) {
    CONTEXT(DEBUG, "Exited error state");
    return ARRAY_POP(*parser, errorStack);
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
        Error* err = errNew(ERROR_SYNTAX);
        errAddText(err, TextRed, "Unexpected Character %s",
            parser->current.data.string);
        errAddSource(err, &parser->current.range);
        errEmit(err, parser);
    }
}

// consume a token of type type, else return false
static bool match(Parser* parser, MicrocodeTokenType type) {
    CONTEXT(DEBUG, "Parser token match");
    if(!check(parser, type)) return false;
    advance(parser);
    return true;
}

static void consume(Parser* parser, MicrocodeTokenType tok,
    const char* message, ...)
{
    CONTEXT(DEBUG, "consume valid token");

    va_list args;
    va_start(args, message);

    if(parser->current.type == tok) {
        advance(parser);
        va_end(args);
        DEBUG("Found %s token", TokenNames[tok]);
        return;
    }

    INFO("Could not find %s token", TokenNames[tok]);

    Error* err = errNew(ERROR_SYNTAX);
    vErrAddText(err, TextRed, message, args);
    errAddSource(err, &parser->current.range);
    errEmit(err, parser);

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
            case TOKEN_TYPE:
            case TOKEN_BITGROUP:
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

static ASTBitArray parseMicrocodeBitArray(Parser* parser) {
    CONTEXT(INFO, "Parsing microcode bit array");
    ASTBitArray result;
    result.range = parser->current.range;
    ARRAY_ALLOC(ASTBit, result, data);
    while(match(parser, TOKEN_IDENTIFIER)) {
        ASTBit bit;
        bit.data = parser->previous;
        bit.range = bit.data.range;
        ARRAY_ZERO(bit, param);
        if(match(parser, TOKEN_LEFT_PAREN)) {
            ARRAY_ALLOC(ASTBitParameter, bit, param);
            while(match(parser, TOKEN_IDENTIFIER)) {
                ASTBitParameter param;
                param.name = parser->previous;
                ARRAY_PUSH(bit, param, param);
                if(!match(parser, TOKEN_COMMA)) {
                    break;
                }
            }
            consume(parser, TOKEN_RIGHT_PAREN, "Expected ')', got '%s'",
                parser->previous.data.string);
        }
        bit.range.length = parser->previous.range.tokenStart +
            parser->previous.range.length - bit.range.tokenStart;
        ARRAY_PUSH(result, data, bit);

        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }
    result.range.length = parser->previous.range.tokenStart +
        parser->previous.range.length - result.range.tokenStart;
    return result;
}

// parses a line of microcode commands with conditions
// returns the line ast representing what was parsed
static ASTMicrocodeLine* microcodeLine(Parser* parser) {
    CONTEXT(INFO, "Parsing single microcode line");
    ASTMicrocodeLine* line = ArenaAlloc(sizeof(ASTMicrocodeLine));
    line->range = parser->current.range;

    line->bits = parseMicrocodeBitArray(parser);

    line->range.length = parser->previous.range.tokenStart +
        parser->previous.range.length - line->range.tokenStart;

    return line;
}

static void typeEnum(Parser* parser, ASTStatement* s) {
    CONTEXT(INFO, "Parsing enum type expression");

    s->as.type.type = AST_TYPE_STATEMENT_ENUM;

    consume(parser, TOKEN_LEFT_PAREN, "Expected \"(\" before width of enum");
    consume(parser, TOKEN_NUMBER, "Expected enum width to be a number");
    s->as.type.as.enumType.width = parser->previous;

    consume(parser, TOKEN_RIGHT_PAREN, "Expected \")\" after enum width");
    consume(parser, TOKEN_LEFT_BRACE, "Expected \"{\" at start of block");

    ARRAY_ALLOC(Token, s->as.type.as.enumType, member);
    while(match(parser, TOKEN_IDENTIFIER)) {
        ARRAY_PUSH(s->as.type.as.enumType, member, parser->previous);
        if(!match(parser, TOKEN_SEMICOLON)) {
            break;
        }
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expected \"}\" at end of block");
}

static void type(Parser* parser) {
    CONTEXT(INFO, "Parsing new type declaration");
    newErrorState(parser);

    ASTStatement* s = newStatement(parser, AST_BLOCK_TYPE);

    consume(parser, TOKEN_IDENTIFIER, "Expected name of type being parsed");
    s->as.type.name = parser->previous;

    consume(parser, TOKEN_EQUAL,
        "Expected \"=\" to assign value in type declaration");

    advance(parser);
    switch(parser->previous.type) {
        case TOKEN_ENUM: typeEnum(parser, s); break;
        default: {
            Error* err = errNew(ERROR_SYNTAX);
            errAddText(err, TextRed, "Expected type name, got %s");
            errAddSource(err, &parser->previous.range);
            errEmit(err, parser);
        }
    }

    s->isValid = !endErrorState(parser);
}

static void parameter(Parser* parser) {
    CONTEXT(INFO, "Parsing new parameter");
    newErrorState(parser);

    ASTStatement* s = newStatement(parser, AST_BLOCK_PARAMETER);
    Token* name = &parser->previous;
    s->as.parameter.name = *name;

    consume(parser, TOKEN_COLON,
        "Missing colon seperating property %s from its value",
        name->data.string);
    consume(parser, TOKEN_NUMBER, "Missing value of %s parameter",
        name->data.string);
    s->as.parameter.value = parser->previous;

    s->isValid = !endErrorState(parser);
}

// parses a header statement
static void header(Parser* parser) {
    CONTEXT(INFO, "Parsing header statement");
    newErrorState(parser);

    ASTStatement* s = newStatement(parser, AST_BLOCK_HEADER);
    ARRAY_ALLOC(ASTBitArray, s->as.header, line);
    s->as.header.errorPoint = parser->previous;

    consume(parser, TOKEN_LEFT_BRACE, "Expected \"{\" at start of block");

    while(!check(parser, TOKEN_EOF)) {
        if(check(parser, TOKEN_RIGHT_BRACE)) {
            break;
        }
        ASTMicrocodeLine* line = microcodeLine(parser);

        ARRAY_PUSH(s->as.header, line, line->bits);
        if(!match(parser, TOKEN_SEMICOLON)) {
            break;
        }
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expected \"}\" at end of block");

    s->isValid = !endErrorState(parser);
}

static void opcode(Parser* parser) {
    CONTEXT(INFO, "Parsing opcode statement");
    newErrorState(parser);

    ASTStatement* s = newStatement(parser, AST_BLOCK_OPCODE);
    ARRAY_ALLOC(ASTMicrocodeLine*, s->as.opcode, line);

    s->as.opcode.range = parser->previous.range;

    consume(parser, TOKEN_IDENTIFIER, "Expected opcode name, got %s",
        TokenNames[parser->current.type]);
    s->as.opcode.name = parser->previous;
    consume(parser, TOKEN_BINARY, "Expected opcode number, got %s",
        TokenNames[parser->current.type]);
    s->as.opcode.id = parser->previous;

    ARRAY_ALLOC(ASTStatementParameter, s->as.opcode, param);
    consume(parser, TOKEN_LEFT_PAREN, "Expected left paren, got %s",
        TokenNames[parser->current.type]);
    while(match(parser, TOKEN_IDENTIFIER)) {
        ASTStatementParameter param = {0};
        param.name = parser->previous;

        consume(parser, TOKEN_IDENTIFIER,
            "Expecting the name of a parameter after its type");
        param.value = parser->previous;

        ARRAY_PUSH(s->as.opcode, param, param);

        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }
    consume(parser, TOKEN_RIGHT_PAREN, "Expected right paren, got %s",
        TokenNames[parser->current.type]);


    INFO("Parsed opcode statement header");

    consume(parser, TOKEN_LEFT_BRACE, "Expected \"{\" at start of block");

    DEBUG("Parsing long opcode statement");
    while(!check(parser, TOKEN_EOF)) {
        if(check(parser, TOKEN_RIGHT_BRACE)) {
            break;
        }
        ASTMicrocodeLine* line = microcodeLine(parser);
        ARRAY_PUSH(s->as.opcode, line, line);
        if(!match(parser, TOKEN_SEMICOLON)) {
            break;
        }
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expected \"}\" at end of block");

    s->as.opcode.range.length = parser->previous.range.tokenStart + parser->previous.range.length - s->as.opcode.range.tokenStart;

    s->isValid = !endErrorState(parser);
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
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Could not find file \"%s\"",
                parser->previous.data.string);
            errAddSource(err, &parser->previous.range);
            errEmit(err, parser);
            return;
        }

        INFO("Found file to include, parsing");
        Scanner* newScanner = ArenaAlloc(sizeof(Scanner));
        Parser* newParser = ArenaAlloc(sizeof(Parser));
        ScannerInit(newScanner, readFilePtr(file), foundFileName);
        Parse(newParser, newScanner, parser->ast);

        if(newParser->hadError){
            parser->hadError = true;
            for(unsigned int i = 0; i < newParser->errorCount; i++) {
                ARRAY_PUSH(*parser, error, newParser->errors[i]);
            }
        }

        INFO("Include parse completed");
    } else {
        INFO("Could not find input file string in source");
        Error* err = errNew(ERROR_SYNTAX);
        errAddText(err, TextRed,
            "Expected string containing file name to include.");
        errAddSource(err, &parser->current.range);
    }
}

static void bitgroup(Parser* parser) {
    CONTEXT(INFO, "Parsing bitgroup statement");
    newErrorState(parser);

    ASTStatement* s = newStatement(parser, AST_BLOCK_BITGROUP);

    consume(parser, TOKEN_IDENTIFIER, "A bitgroup statement requires a name");
    s->as.bitGroup.name = parser->previous;

    ARRAY_ALLOC(ASTStatementParameter, s->as.bitGroup, param);
    consume(parser, TOKEN_LEFT_PAREN, "Expected left paren, got %s",
        TokenNames[parser->current.type]);
    while(match(parser, TOKEN_IDENTIFIER)) {
        ASTStatementParameter param = {0};
        param.name = parser->previous;

        consume(parser, TOKEN_IDENTIFIER,
            "Expecting the name of a parameter after its type");
        param.value = parser->previous;

        ARRAY_PUSH(s->as.bitGroup, param, param);

        if(!match(parser, TOKEN_COMMA)) {
            break;
        }
    }
    consume(parser, TOKEN_RIGHT_PAREN, "Expected right paren, got %s",
        TokenNames[parser->current.type]);

    ARRAY_ALLOC(ASTBitGroupIdentifier, s->as.bitGroup, segment);
    consume(parser, TOKEN_LEFT_BRACE, "Expected \"{\" at start of block");
    while(!match(parser, TOKEN_EOF)) {
        ASTBitGroupIdentifier id;

        if(match(parser, TOKEN_RIGHT_BRACE)) {
            break;
        } else if(match(parser, TOKEN_DOLLAR)) {
            id.type = AST_BIT_GROUP_IDENTIFIER_SUBST;
            consume(parser, TOKEN_LEFT_PAREN, "Expected left paren");
            consume(parser, TOKEN_IDENTIFIER,
                "A bitgroup statement requires a name");
            id.identifier = parser->previous;
            consume(parser, TOKEN_RIGHT_PAREN, "Expected right paren");
        } else if(match(parser, TOKEN_IDENTIFIER)) {
            id.type = AST_BIT_GROUP_IDENTIFIER_LITERAL;
            id.identifier = parser->previous;
        } else {
            advance(parser);
            Error* err = errNew(ERROR_SYNTAX);
            errAddText(err, TextRed, "Unexpected token of type %s while "
                "parsing bitgroup. Expecting identifier or '$'",
                TokenNames[parser->previous.type]);
            errAddSource(err, &parser->previous.range);
            errEmit(err, parser);
            break;
        }
        ARRAY_PUSH(s->as.bitGroup, segment, id);
    }

    s->isValid = !endErrorState(parser);
}

// dispatch the parser for a block level statement
static void block(Parser* parser) {
    CONTEXT(INFO, "Parsing block statement");
    advance(parser);

    switch(parser->previous.type) {
        case TOKEN_TYPE: type(parser); break;
        case TOKEN_OPCODE: opcode(parser); break;
        case TOKEN_HEADER: header(parser); break;
        case TOKEN_INCLUDE: include(parser); break;
        case TOKEN_BITGROUP: bitgroup(parser); break;
        case TOKEN_IDENTIFIER: parameter(parser); break;
        default: {
            INFO("Could not find valid block statement");
            Error* err = errNew(ERROR_SYNTAX);
            errAddText(err, TextRed, "Expected a block statement, got %s",
                TokenNames[parser->previous.type]);
            errAddSource(err, &parser->previous.range);
            errEmit(err, parser);
        }
    }

    // if error occured reset parser state to known value
    if(parser->panicMode) syncronise(parser);
}

static bool runParser(Parser* parser) {
    CONTEXT(INFO, "Running microcode parser");

    // gets first token, required so not matching garbage(causes segfault)
    advance(parser);

    DEBUG("Checking filename is valid");
    for(unsigned int i = 0; i < parser->ast->fileNameCount; i++) {
        if(parser->ast->fileNames[i] == parser->scanner->fileName) {
            Error* err = errNew(ERROR_SEMANTIC);
            errAddText(err, TextRed, "Recursive include detected of file "
                "\"%s\"", parser->scanner->fileName);
            errAddSource(err, &parser->current.range);
            errEmit(err, parser);
            return false;
        }
    }
    ARRAY_PUSH(*parser->ast, fileName, parser->scanner->fileName);
    DEBUG("Filename checks passed");

    INFO("Starting parsing");
    while(!match(parser, TOKEN_EOF)){
        // all file-level constructs are blocks of some form
        block(parser);
    }

    INFO("Reached end of input file");
    return !parser->hadError;
}

bool Parse(Parser* parser, Scanner* scan, AST* ast) {
    CONTEXT(INFO, "Initialising new parser");

    parser->scanner = scan;
    parser->hadError = false;
    parser->panicMode = false;
    ARRAY_ALLOC(bool, *parser, errorStack);
    ARRAY_ALLOC(Error*, *parser, error);
    parser->ast = ast;

    return runParser(parser);
}