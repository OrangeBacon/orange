#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shared/memory.h"
#include "shared/platform.h"
#include "microcode/parser.h"
#include "microcode/error.h"
#include "shared/log.h"

/*
static void printLine(Parser* parser, int line, int* start, int* length, int lineNumberLength) {
    if(getLine(parser->scanner->base, line, start, length)) {
        cErrPrintf(TextWhite, "  %*i | %.*s\n", lineNumberLength, line, *length, parser->scanner->base + *start);
    }
}

// print a message about a token to stderr
static void printMessage(Parser* parser, SourceRange* range, const char* name, unsigned int code, TextColor color,
    const char* message) {
    CONTEXT(DEBUG, "Printing Error");
    setErrorState(parser);

    // error message
    if(code == 0) {
        cErrPrintf(color, "%s: ", name);
    } else {
        cErrPrintf(color, "%s [E%04u]: ", name, code);
    }
    cErrPrintf(color, message);
    printf("\n");

    // file name/location
    cErrPrintf(TextWhite, "  --> %s:%i:%i\n", parser->scanner->fileName, range->line, range->column);

    // number of charcters required to print the longest line number
    int lineNumberLength = floor(log10(abs(range->line + 1))) + 1;

    int start;
    int length;

    // line before the error
    printLine(parser, range->line - 1, &start, &length, lineNumberLength);

    // line with the error token on
    printLine(parser, range->line, &start, &length, lineNumberLength);

    // how far along the line the error token starts
    int startPos = range->start - parser->scanner->base - start;

    // buffer to store arrow to errored token
    char* buf = malloc(length * sizeof(char));
    if(buf != NULL) {
        // space fill
        for(int i = 0; i < length; i++) {
            buf[i] = ' ';
        }
        // underline fill length below errored token
        for(int i = startPos; i < startPos + range->length; i++) {
            buf[i] = '^';
        }

        cErrPrintf(TextWhite, "  %*s | ", lineNumberLength, "");

        cErrPrintf(color, "%.*s\n", length, buf);
    }

    // line after error token
    printLine(parser, range->line + 1, &start, &length, lineNumberLength);

    printf("\n");
}*/

Error* errNew(ErrorLevel level) {
    CONTEXT(INFO, "Creating Error");
    Error* err = ArenaAlloc(sizeof(Error));

    err->level = level;
    ARRAY_ALLOC(ErrorChunk, *err, chunk);

    return err;
}

void errAddText(Error* err, TextColor color, const char* text, ...) {
    va_list args;
    va_start(args, text);
    vErrAddText(err, color, text, args);
    va_end(args);
}

void vErrAddText(Error* err, TextColor color, const char* text, va_list args) {
    CONTEXT(TRACE, "Adding text to error");
    ErrorChunk chunk;
    chunk.type = ERROR_CHUNK_TEXT;
    chunk.as.text.message = vaprintf(text, args);
    chunk.as.text.color = color;
    PUSH_ARRAY(ErrorChunk, *err, chunk, chunk);
}

void errAddSource(Error* err, SourceRange* loc) {
    CONTEXT(TRACE, "Adding source to error");
    ErrorChunk chunk;
    chunk.type = ERROR_CHUNK_SOURCE;
    chunk.as.source = *loc;
    PUSH_ARRAY(ErrorChunk, *err, chunk, chunk);
}

void errEmit(Error* err, struct Parser* parser) {
    CONTEXT(INFO, "Emitting created error");
    if(parser->panicMode) return;
    parser->hadError = true;

    if(err->level == ERROR_SYNTAX) {
        parser->panicMode = true;
    }

    PUSH_ARRAY(Error, *parser, error, err);
}

void printErrors(Parser* parser) {
    printf("PRINTING ERROR - TODO: SHOW PROPER INFOMATION");
    (void)parser;
}
