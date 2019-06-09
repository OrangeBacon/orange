#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "parser.h"
#include "platform.h"
#include "error.h"

bool printLine(Parser* parser, int line, int* start, int* length, int maxLineLength, int lineNumberLength) {
    bool s = getLine(parser->scanner->base, line, start, length);
    if(s) {
        if(*length > maxLineLength) {
            cErrPrintf(TextWhite, "  %*i | %.*s", lineNumberLength, line, maxLineLength - 5, parser->scanner->base + *start);
            cErrPrintf(TextYellow, " ...\n");
        } else {
            cErrPrintf(TextWhite, "  %*i | %.*s\n", lineNumberLength, line, *length, parser->scanner->base + *start);
        }
    }
    return s;
}

// print a message about a token to stderr
// assumes the token is correctly formed
// and is all on one line
void printMessage(Parser* parser, Token* token, const char* name, TextColor color, const char* message, va_list args) {
    // error message
    cErrPrintf(color, "%s: ", name);
    cErrVPrintf(color, message, args);
    printf("\n");

    // file name/location
    cErrPrintf(TextWhite, "  --> %s:%i:%i\n", parser->ast.fileName, token->line, token->column);

    // number of charcters required to print the longest line number
    int lineNumberLength = floor(log10(abs(token->line + 1))) + 1;

    // how many source code characters can be printed
    int maxLineLength = 80 - (2 + lineNumberLength + 3);
    // 80 = terminal width
    // 2 = spacing
    // 3 = spacing and pipe symbol

    int start;
    int length;

    // line before the error
    printLine(parser, token->line - 1, &start, &length, maxLineLength, lineNumberLength);

    // line with the error token on
    printLine(parser, token->line, &start, &length, maxLineLength, lineNumberLength);
    
    // how far along the line the error token starts
    int startPos = token->start - parser->scanner->base - start;

    // min (how long the error token's line is, maximum line length)
    int lineLen = length > maxLineLength ? maxLineLength : length;

    // buffer to store arrow to errored token
    char* buf = malloc(length * sizeof(char));
    if(buf != NULL) {
        // space fill
        for(int i = 0; i < length; i++) {
            buf[i] = ' ';
        }
        // underline fill length below errored token
        for(int i = startPos; i < startPos + token->length; i++) {
            buf[i] = '^';
        }

        cErrPrintf(TextWhite, "  %*s | ", lineNumberLength, "");

        // print error arrow or truncation if it does not fit
        if(startPos <= maxLineLength) {
            cErrPrintf(color, "%.*s\n", lineLen, buf);
        } else {
            cErrPrintf(color, "%*s\n",lineLen , "...>");
        }
    }

    // line after error token
    printLine(parser, token->line + 1, &start, &length, maxLineLength, lineNumberLength);

    printf("\n");
}

// print an error message at a token's position
bool vErrorAt(Parser* parser, Token* token, const char* message, va_list args) {
    if(parser->panicMode) return false;
    parser->panicMode = true;
    printMessage(parser, token, "Error", TextRed, message, args);
    parser->hadError = true;
    parser->ast.hasError = true;
    return true;
}

// print a note relating to an error token
bool vNoteAt(Parser* parser, Token* token, const char* message, va_list args) {
    printMessage(parser, token, "Note", TextBlue, message, args);
    return true;
}

// print a warning relating to a token
bool vWarnAt(Parser* parser, Token* token, const char* message, va_list args) {
    if(parser->panicMode) return false;
    printMessage(parser, token, "Warn", TextMagenta, message, args);
    parser->hadError = true;
    parser->ast.hasError = true;
    return true;
}

// print an error message at a token's position
bool errorAt(Parser* parser, Token* token, const char* message, ...) {
    va_list args;
    va_start(args, message);
    return vErrorAt(parser, token, message, args);
}

// print a note relating to an error token
bool noteAt(Parser* parser, Token* token, const char* message, ...) {
    va_list args;
    va_start(args, message);
    return vNoteAt(parser, token, message, args);
}

// print a warning relating to a token
bool warnAt(Parser* parser, Token* token, const char* message, ...) {
    va_list args;
    va_start(args, message);
    return vWarnAt(parser, token, message, args);
}

// issue error for token before advance() called
bool vErrorAtCurrent(Parser* parser, const char* message, va_list args) {
    return vErrorAt(parser, &parser->current, message, args);
}

// issue error for token before advance() called
bool errorAtCurrent(Parser* parser, const char* message, ...) {
    va_list args;
    va_start(args, message);
    return vErrorAtCurrent(parser, message, args);
}

// issue warning for already advanced() token
bool warn(Parser* parser, const char* message, ...) {
    va_list args;
    va_start(args, message);
    return vWarnAt(parser, &parser->previous, message, args);
}