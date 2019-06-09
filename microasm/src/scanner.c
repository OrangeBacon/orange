#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

static char peek(Scanner* scanner);
static char advance(Scanner* scanner);
static bool isAtEnd(Scanner* scanner);
static bool isDigit(char c);
static bool isIdent(char c);
static void skipWhitespace(Scanner* scanner);
static Token number(Scanner* scanner);
static Token identifier(Scanner* scanner);
static OrangeTokenType identifierType(Scanner* scanner);
static OrangeTokenType checkKeyword(Scanner* scanner, int start, int length, 
    const char* rest, OrangeTokenType type);
static Token makeToken(Scanner* scanner, OrangeTokenType type);
static Token errorToken(Scanner* scanner, const char* message);

void ScannerInit(Scanner* scanner, const char* source, const char* fileName) {
    scanner->line = 1;
    scanner->column = 1;
    scanner->current = source;
    scanner->start = source;
    scanner->base = source;
    scanner->fileName = fileName;
}

Token ScanToken(Scanner* scanner){
    // all whitespace is insignificant at the start of a token
    skipWhitespace(scanner);

    // scan current token from the end of the last token + whitespace
    scanner->start = scanner->current;

    if(isAtEnd(scanner)){
        scanner->column++; // so EOF is in the column after the last token
        return makeToken(scanner, TOKEN_EOF);
    }

    char c = advance(scanner);

    if(isDigit(c)) return number(scanner);
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

bool getLine(const char* string, int line, int* start, int* length) {
    // line that is being iterated over
    int currentLine = 1;

    // should the iteration stop at the next newline character
    bool willEnd = false;
    
    // length of the line that is being iterated over
    int lineLength = 0;

    for(int i = 0; string[i] != '\0'; i++) {
        if(string[i] == '\n') {

            // handle the line being requested being the first line
            // which just ended
            if(line == currentLine && line == 1) {
                *start = 0;
                *length = lineLength;
                return true;
            }
            currentLine++;

            // got to end of requested line
            if(willEnd) {
                *length = lineLength-1;
                return true;
            }

            // start iterating over requested line
            if(currentLine == line && !willEnd) {
                *start = i+1;
                willEnd = true;
                lineLength = 1;
            }
        } else {
            lineLength++;
        }
    }

    // handle the line requested being the last line of the string
    if(willEnd) {
        *length = lineLength;
        return true;
    }

    // line requested not found in string given
    return false;
}

// get next character without advancing the stream
static char peek(Scanner* scanner) {
    return *scanner->current;
}

// get the next character in the stream
static char advance(Scanner* scanner) {
    if(isAtEnd(scanner)) return '\0';
    scanner->current++;
    scanner->column++;
    return scanner->current[-1];
}

// are there more characters to read?
static bool isAtEnd(Scanner* scanner) {
    return *scanner->current == '\0';
}

// is the character numerical?
static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// is the character a hexadecimal character
static bool isHexDigit(char c) {
    return (c >='0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// is the character not whitespace and not part of any other token?
static bool isIdent(char c) {
    return !(c =='(' || c == ')' || c == '{' ||
        c == '}' || c == ';' || c == ':' || c == '=' ||
        c == '*' || c == ',' || c == ' ' || c == '\r' ||
        c == '\n' || c == '#');
}

// ignore any ' ', '\t', '\r', '\n' and comments
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
                scanner->column = 0;
                advance(scanner);
                break;
            case '#':
                // skip comment until just before end of line
                while(peek(scanner) != '\n' && !isAtEnd(scanner)) {
                    advance(scanner);
                }
                break;
            default:
                return;
        }
    }
}

// scan a number
static Token number(Scanner* scanner) {
    while(!isAtEnd(scanner)) {
        if(!isDigit(peek(scanner))) {
            break;
        }
        advance(scanner);
    }
    if(scanner->current == scanner->start + 1 && scanner->current[-1] == '0' && peek(scanner) =='x') {
        advance(scanner);
        while(!isAtEnd(scanner)) {
            if(!isHexDigit(peek(scanner))) {
                break;
            }
            advance(scanner);
        }
    }
    return makeToken(scanner, TOKEN_NUMBER);
}

// scan an identifier
static Token identifier(Scanner* scanner) {
    while(isIdent(peek(scanner))) {
        advance(scanner);
    }

    return makeToken(scanner, identifierType(scanner));
}

// what is the token type of the last identifier scanned?
static OrangeTokenType identifierType(Scanner* scanner) {
    // uses a trie - no keywords begin with anything other
    // than 'h' 'm' 'i' 'o', etc.
    switch(scanner->start[0]) {
        case 'h': return checkKeyword(scanner, 1, 5, "eader", TOKEN_HEADER);
        case 'm': return checkKeyword(scanner, 1, 4, "acro", TOKEN_MACRO);
        case 'i': return checkKeyword(scanner, 1, 4, "nput", TOKEN_INPUT);
        case 'o': if(scanner->current - scanner->start > 1) {
            switch(scanner->start[1]) {
                case 'p': return checkKeyword(scanner, 2, 4, "code", TOKEN_OPCODE);
                case 'u': return checkKeyword(scanner, 2, 4, "tput", TOKEN_OUTPUT);
            }
        }
    }
    return TOKEN_IDENTIFIER;
}

// is the remainder of the last scanned identifier the same as the provided string
static OrangeTokenType checkKeyword(Scanner* scanner, int start, int length, 
      const char* rest, OrangeTokenType type) {
    if(scanner->current - scanner->start == start + length && 
          memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// create a token based on the currently advanced characters
static Token makeToken(Scanner* scanner, OrangeTokenType type) {
    Token token;
    token.type = type;
    token.base = scanner->base;
    token.offset = (int)(scanner->start - scanner->base);
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    token.column = scanner->column;

    return token;
}

// create error which will be displayed by the parser
static Token errorToken(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.base = scanner->base;
    token.offset = (int)(scanner->start - scanner->base);
    token.length = (int)strlen(message);
    token.line = scanner->line;
    token.column = scanner->column;

    return token;
}