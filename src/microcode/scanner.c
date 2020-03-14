#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "microcode/scanner.h"
#include "shared/memory.h"
#include "shared/platform.h"

static char peek(Scanner* scanner);
static char advance(Scanner* scanner);
static bool isAtEnd(Scanner* scanner);
static bool isDigit(char c);
static bool isIdent(char c);
static void skipWhitespace(Scanner* scanner);
static Token number(Scanner* scanner);
static Token string(Scanner* scanner, char end);
static Token identifier(Scanner* scanner);
static MicrocodeTokenType identifierType(Scanner* scanner);
static MicrocodeTokenType checkKeyword(Scanner* scanner, int start, int length,
    const char* rest, MicrocodeTokenType type);
static Token makeToken(Scanner* scanner, MicrocodeTokenType type);
static Token errorToken(Scanner* scanner, const char* message);

void ScannerInit(Scanner* scanner, const char* source, const char* fileName) {
    scanner->line = 1;
    scanner->column = 1;
    scanner->current = source;
    scanner->start = source;
    scanner->base = source;
    scanner->fileName = resolvePath(fileName);
}

static bool match(Scanner* scanner, char expected) {
    if(isAtEnd(scanner)) return false;
    if(*scanner->current != expected) return false;

    scanner->current++;
    scanner->column++;

    return true;
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
        case ',': return makeToken(scanner, TOKEN_COMMA);
        case '$': return makeToken(scanner, TOKEN_DOLLAR);
        case '!': return makeToken(scanner, match(scanner, '=')?
            TOKEN_EXCLAIM_EQUAL : TOKEN_EXCLAMATION);
        case '=': return makeToken(scanner, match(scanner, '=')?
            TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '"': return string(scanner, '"');
        case '\'': return string(scanner, '\'');
    }

    return errorToken(scanner, "Unexpected character");
}

int getLineCount(const char* string) {
    int lineCount = 1;

    for(int i = 0; string[i] != '\0'; i++) {
        if(string[i] == '\n') {
            lineCount++;
        }
    }

    return lineCount;
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
static bool isBinaryDigit(char c) {
    return (c == '0') || (c == '1');
}

// is the character not whitespace and not part of any other token?
static bool isIdent(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
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
    bool bin = false;
    while(!isAtEnd(scanner)) {
        if(!isDigit(peek(scanner))) {
            break;
        }
        advance(scanner);
    }
    if(scanner->current == scanner->start + 1 && scanner->current[-1] == '0' && peek(scanner) =='b') {
        bin = true;
        advance(scanner);
        while(!isAtEnd(scanner)) {
            if(!isBinaryDigit(peek(scanner))) {
                break;
            }
            advance(scanner);
        }
    }

    char* endPtr;
    long val;
    if(bin) {
        val = strtol(scanner->start + 2, &endPtr, 2);
    } else {
        val = strtol(scanner->start, &endPtr, 10);
    }
    if(val > INT_MAX) {
        return errorToken(scanner, "Number too large for integer type");
    }

    Token out = makeToken(scanner, bin ? TOKEN_BINARY : TOKEN_NUMBER);
    out.data.value = val;
    return out;
}

static Token string(Scanner* scanner, char end) {
    while(!isAtEnd(scanner)) {
        char c = advance(scanner);
        if(c == end) {
            break;
        }
    }

    Token ret = makeToken(scanner, TOKEN_STRING);
    char* buf = ArenaAlloc(sizeof(char) * (ret.range.length-2+1));
    *buf = '\0';
    strncat(buf, ret.range.tokenStart+1, ret.range.length-2);
    ret.data.string = buf;
    return ret;
}

// scan an identifier
static Token identifier(Scanner* scanner) {
    while(isIdent(peek(scanner))) {
        advance(scanner);
    }

    return makeToken(scanner, identifierType(scanner));
}

// what is the token type of the last identifier scanned?
static MicrocodeTokenType identifierType(Scanner* scanner) {
    // uses a trie
    switch(scanner->start[0]) {
        case 'b': return checkKeyword(scanner, 1, 7, "itgroup", TOKEN_BITGROUP);
        case 'e': return checkKeyword(scanner, 1, 3, "num", TOKEN_ENUM);
        case 'h': return checkKeyword(scanner, 1, 5, "eader", TOKEN_HEADER);
        case 'i': return checkKeyword(scanner, 1, 6, "nclude", TOKEN_INCLUDE);
        case 'o': return checkKeyword(scanner, 1, 5, "pcode", TOKEN_OPCODE);
        case 't':
            if(scanner->start[1] == 'y') {
                return checkKeyword(scanner, 2, 2, "pe", TOKEN_TYPE);
            } else if(scanner->start[1] == 'r') {
                return checkKeyword(scanner, 2, 2, "ue", TOKEN_TRUE);
            }
            break;
        case 'f': return checkKeyword(scanner, 1, 4, "alse", TOKEN_FALSE);
    }
    return TOKEN_IDENTIFIER;
}

// is the remainder of the last scanned identifier the same as the provided string
static MicrocodeTokenType checkKeyword(Scanner* scanner, int start, int length,
      const char* rest, MicrocodeTokenType type) {
    if(scanner->current - scanner->start == start + length &&
          memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// create a token based on the currently advanced characters
static Token makeToken(Scanner* scanner, MicrocodeTokenType type) {
    Token token;
    token.type = type;
    token.range.tokenStart = scanner->start;
    token.range.length = (int)(scanner->current - scanner->start);
    token.range.line = scanner->line;
    token.range.column = scanner->column;
    token.range.filename = scanner->fileName;
    token.range.sourceStart = scanner->base;
    if(type == TOKEN_IDENTIFIER) {
        token.data.string = tokenAllocName(&token);
    }

    return token;
}

// create error which will be displayed by the parser
static Token errorToken(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.range.tokenStart = scanner->start;
    token.range.length = (int)(scanner->current - scanner->start);
    token.range.line = scanner->line;
    token.range.column = scanner->column;
    token.range.filename = scanner->fileName;
    token.range.sourceStart = scanner->base;
    token.data.string = message;

    return token;
}
