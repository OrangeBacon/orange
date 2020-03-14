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

//-------------------//
// expression parser //
//-------------------//
typedef ASTExpression* (*PrefixFn)(Parser*);
typedef ASTExpression* (*InfixFn)(Parser*, ASTExpression*);

typedef struct ParseRule {
    PrefixFn prefix;
    InfixFn infix;
    Precidence precidence;
} ParseRule;

static ParseRule* getRule(MicrocodeTokenType type);

static ASTExpression* parsePrecidence(Parser* parser, Precidence precidence) {
    advance(parser);
    PrefixFn prefixRule = getRule(parser->previous.type)->prefix;
    if(prefixRule == NULL) {
        Error* err = errNew(ERROR_SYNTAX);
        errAddText(err, TextRed, "Expected expression");
        errAddSource(err, &parser->previous.range);
        errEmit(err, parser);
        return NULL;
    }

    ASTExpression* exp = prefixRule(parser);

    while(precidence <= getRule(parser->current.type)->precidence) {
        advance(parser);
        InfixFn infixRule = getRule(parser->previous.type)->infix;
        exp = infixRule(parser, exp);
    }

    return exp;
}

static ASTExpression* expression(Parser* parser) {
    return parsePrecidence(parser, PREC_COMMA);
}

static ASTExpression* grouping(Parser* parser) {
    ASTExpression* e = expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
    return e;
}

static ASTExpression* number(Parser* parser) {
    ASTExpression* e = ArenaAlloc(sizeof(*e));
    e->type = AST_EXPRESSION_NUMBER;
    e->as.number = parser->previous;
    return e;
}

static ASTExpression* variable(Parser* parser) {
    ASTExpression* e = ArenaAlloc(sizeof(*e));
    e->type = AST_EXPRESSION_VARIABLE;
    e->as.variable = parser->previous;
    return e;
}

static ASTExpression* string(Parser* parser) {
    ASTExpression* e = ArenaAlloc(sizeof(*e));
    e->type = AST_EXPRESSION_STRING;
    e->as.string = parser->previous;
    return e;
}

static ASTExpression* unary(Parser* parser) {
    Token opcode = parser->previous;

    ASTExpression* e = parsePrecidence(parser, PREC_UNARY);

    ASTExpression* unary = ArenaAlloc(sizeof(*unary));
    unary->type = AST_EXPRESSION_UNARY;
    unary->as.unary.opcode = opcode;
    unary->as.unary.operand = e;

    if(opcode.type == TOKEN_EXCLAMATION) {
        unary->as.unary.type = AST_EXPRESSION_UNARY_NOT;
    } else {
        ERROR("Unreachable state reached");
    }

    return unary;
}

static ASTExpression* boolean(Parser* parser) {
    ASTExpression* lit = ArenaAlloc(sizeof(*lit));
    lit->type = AST_EXPRESSION_BOOLEAN;
    lit->as.boolean = parser->previous;

    if(lit->as.boolean.type == TOKEN_TRUE) {
        lit->as.boolean.data.value = 1;
    } else {
        lit->as.boolean.data.value = 0;
    }

    return lit;
}

static ASTExpression* call(Parser* parser, ASTExpression* prev) {
    ASTExpression* call = ArenaAlloc(sizeof(*call));
    call->type = AST_EXPRESSION_CALL;
    call->as.call.callee = prev;
    ARRAY_ALLOC(ASTExpression*, call->as.call, param);

    do {
        ASTExpression* e = parsePrecidence(parser, PREC_OR);
        ARRAY_PUSH(call->as.call, param, e);
    } while(match(parser, TOKEN_COMMA));

    consume(parser, TOKEN_RIGHT_PAREN, "Expected closing ')' after call");

    return call;
}

static ASTExpression* comma(Parser* parser, ASTExpression* prev) {
    ASTExpression* comma = ArenaAlloc(sizeof(*comma));
    comma->type = AST_EXPRESSION_LIST;
    ARRAY_ALLOC(ASTExpression*, comma->as.list, element);
    ARRAY_PUSH(comma->as.list, element, prev);

    do {
        ASTExpression* e = parsePrecidence(parser, PREC_OR);
        ARRAY_PUSH(comma->as.list, element, e);
    } while(match(parser, TOKEN_COMMA));

    return comma;
}

static ASTExpression* binary(Parser* parser, ASTExpression* prev) {
    Token opcode = parser->previous;

    ParseRule* rule = getRule(opcode.type);
    ASTExpression* e = parsePrecidence(parser,
        (Precidence)(rule->precidence + 1));

    ASTExpression* binary = ArenaAlloc(sizeof(*binary));
    binary->type = AST_EXPRESSION_BINARY;
    binary->as.binary.opcode = opcode;
    binary->as.binary.left = prev;
    binary->as.binary.right = e;

    switch(opcode.type) {
        case TOKEN_EXCLAIM_EQUAL:
            binary->as.binary.type = AST_EXPRESSION_BINARY_NOT_EQUAL;
            break;
        case TOKEN_EQUAL_EQUAL:
            binary->as.binary.type = AST_EXPRESSION_BINARY_EQUAL;
            break;
        default:
            ERROR("Unreachable state reached");
    }

    return binary;
}

ParseRule rules[] = {
    //[Token type] =        { prefix,   infix,  infix precidence}
    [TOKEN_LEFT_PAREN] =    { grouping, call,   PREC_CALL},
    [TOKEN_RIGHT_PAREN] =   { NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE] =    { NULL,     NULL,   PREC_NONE},
    [TOKEN_RIGHT_BRACE] =   { NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA] =         { NULL,     comma,  PREC_COMMA},
    [TOKEN_DOT] =           { NULL,     NULL,   PREC_NONE},
    [TOKEN_COLON] =         { NULL,     NULL,   PREC_NONE},
    [TOKEN_NUMBER] =        { number,   NULL,   PREC_NONE},
    [TOKEN_SEMICOLON] =     { NULL,     NULL,   PREC_NONE},
    [TOKEN_BINARY] =        { NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL] =         { NULL,     NULL,   PREC_NONE},
    [TOKEN_IDENTIFIER] =    { variable, NULL,   PREC_NONE},
    [TOKEN_OPCODE] =        { NULL,     NULL,   PREC_NONE},
    [TOKEN_HEADER] =        { NULL,     NULL,   PREC_NONE},
    [TOKEN_INCLUDE] =       { NULL,     NULL,   PREC_NONE},
    [TOKEN_TYPE] =          { NULL,     NULL,   PREC_NONE},
    [TOKEN_STRING] =        { string,   NULL,   PREC_NONE},
    [TOKEN_ENUM] =          { NULL,     NULL,   PREC_NONE},
    [TOKEN_BITGROUP] =      { NULL,     NULL,   PREC_NONE},
    [TOKEN_DOLLAR] =        { NULL,     NULL,   PREC_NONE},
    [TOKEN_ERROR] =         { NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF] =           { NULL,     NULL,   PREC_NONE},
    [TOKEN_EXCLAMATION] =   { unary,     NULL,  PREC_NONE},
    [TOKEN_EQUAL_EQUAL] =   { NULL,     binary, PREC_EQUALITY},
    [TOKEN_EXCLAIM_EQUAL] = { NULL,     binary, PREC_EQUALITY},
    [TOKEN_OR] =            { NULL,     NULL,   PREC_NONE},
    [TOKEN_AND] =           { NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE] =          { boolean,  NULL,   PREC_NONE},
    [TOKEN_FALSE] =         { boolean,  NULL,   PREC_NONE},
    [TOKEN_NULL] =          { NULL,     NULL,   PREC_NONE},
};

static ParseRule* getRule(MicrocodeTokenType type) {
    return &rules[type];
}

//-------------------//
// statement parsing //
//-------------------//

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
    s->as.parameter.value = expression(parser);

    s->isValid = !endErrorState(parser);
}

// parses a header statement
static void header(Parser* parser) {
    CONTEXT(INFO, "Parsing header statement");
    newErrorState(parser);

    ASTStatement* s = newStatement(parser, AST_BLOCK_HEADER);
    ARRAY_ALLOC(ASTExpression*, s->as.header, expression);
    s->as.header.errorPoint = parser->previous;

    consume(parser, TOKEN_LEFT_BRACE, "Expected \"{\" at start of block");

    while(!check(parser, TOKEN_EOF)) {
        if(check(parser, TOKEN_RIGHT_BRACE)) {
            break;
        }

        ASTExpression* e = expression(parser);

        ARRAY_PUSH(s->as.header, expression, e);
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
    ARRAY_ALLOC(ASTExpression*, s->as.opcode, expression);

    s->as.opcode.range = parser->previous.range;

    consume(parser, TOKEN_IDENTIFIER, "Expected opcode name, got %s",
        TokenNames[parser->current.type]);
    s->as.opcode.name = parser->previous;
    consume(parser, TOKEN_BINARY, "Expected opcode number, got %s",
        TokenNames[parser->current.type]);
    s->as.opcode.id = parser->previous;

    ARRAY_ALLOC(ASTFunctionParameter, s->as.opcode, param);
    consume(parser, TOKEN_LEFT_PAREN, "Expected left paren, got %s",
        TokenNames[parser->current.type]);
    while(match(parser, TOKEN_IDENTIFIER)) {
        ASTFunctionParameter param = {0};
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
        ASTExpression* e = expression(parser);
        ARRAY_PUSH(s->as.opcode, expression, e);
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

    ARRAY_ALLOC(ASTFunctionParameter, s->as.bitGroup, param);
    consume(parser, TOKEN_LEFT_PAREN, "Expected left paren, got %s",
        TokenNames[parser->current.type]);
    while(match(parser, TOKEN_IDENTIFIER)) {
        ASTFunctionParameter param = {0};
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