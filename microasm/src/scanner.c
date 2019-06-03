#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

static char peek(Scanner* scanner);
static char advance(Scanner* scanner);
static bool isAtEnd(Scanner* scanner);
static bool isIdent(char c);
static void skipWhitespace(Scanner* scanner);
static Token identifier(Scanner* scanner);
static TokenType identifierType(Scanner* scanner);
static Token makeToken(Scanner* scanner, TokenType type);
static Token errorToken(Scanner* scanner, const char* message);

void ScannerInit(Scanner* scanner, const char* source) {
    scanner->line = 1;
    scanner->column = 1;
    scanner->current = source;
    scanner->start = source;
}

Token ScanToken(Scanner* scanner){
    skipWhitespace(scanner);

    scanner->start = scanner->current;

    if(isAtEnd(scanner)){
        scanner->column++;
        return makeToken(scanner, TOKEN_EOF);
    }

    char c = advance(scanner);

    if(isIdent(c)) return identifier(scanner);

    switch(c) {
        case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
        case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
        case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
        case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
        case ';': return makeToken(scanner, TOKEN_SEMICOLON);
        case ':': return makeToken(scanner, TOKEN_COLON);
        case '=': return makeToken(scanner, TOKEN_EQUAL);
        case '*': return makeToken(scanner, TOKEN_STAR);
        case ',': return makeToken(scanner, TOKEN_COMMA);
    }

    return errorToken(scanner, "Unexpected character");
}

static char peek(Scanner* scanner) {
    return *scanner->current;
}

static char advance(Scanner* scanner) {
    scanner->current++;
    scanner->column++;
    return scanner->current[-1];
}

static bool isAtEnd(Scanner* scanner) {
    return *scanner->current == '\0';
}

static bool isIdent(char c) {
    return !(c =='(' || c == ')' || c == '{' ||
        c == '}' || c == ';' || c == ':' || c == '=' ||
        c == '*' || c == ',' || c == ' ' || c == '\r' ||
        c == '\n' || c == '#');
}

static void skipWhitespace(Scanner* scanner) {
    for(;;){
        char c = peek(scanner);
        switch(c) {
            case ' ':
            case '\t':
            case '\r':
                advance(scanner);
                break;
            case '\n':
                scanner->line++;
                scanner->column = -1;
                advance(scanner);
                break;
            case '#':
                while(peek(scanner) != '\n' && !isAtEnd(scanner)) {
                    advance(scanner);
                }
                break;
            default:
                return;
        }
    }
}

static Token identifier(Scanner* scanner) {
    while(isIdent(peek(scanner))) {
        advance(scanner);
    }

    return makeToken(scanner, identifierType(scanner));
}
static TokenType identifierType(Scanner* scanner) {
    (void)scanner;
    return TOKEN_IDENTIFIER;
}

static Token makeToken(Scanner* scanner, TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    token.column = scanner->column;

    return token;
}

static Token errorToken(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;
    token.column = scanner->column;

    return token;
}