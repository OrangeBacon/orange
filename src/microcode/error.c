#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shared/memory.h"
#include "shared/platform.h"
#include "microcode/parser.h"
#include "microcode/error.h"
#include "shared/log.h"


static void printLine(SourceRange* range, int lineNumberLength, int offset,
    int* start, int* length)
{
    if(getLine(range->sourceStart, range->line + offset, start, length)) {
        cErrPrintf(TextWhite, "  %*i | %.*s\n", lineNumberLength,
            range->line + offset, *length, range->sourceStart + *start);
    }
}

// print a message about a token to stderr
static void printMessage(SourceRange* range, TextColor color) {
    CONTEXT(DEBUG, "Printing Error Source");

    // file name/location
    cErrPrintf(TextWhite, "  --> %s:%i:%i\n", range->filename, range->line, range->column);

    // number of charcters required to print the longest line number
    int lineNumberLength = floor(log10(abs(range->line + 1))) + 1;

    int start;
    int length;

    // line before the error
    printLine(range, lineNumberLength, -1, &start, &length);

    // line with the error token on
    printLine(range, lineNumberLength, 0, &start, &length);

    // how far along the line the error token starts
    int startPos = range->tokenStart - range->sourceStart - start;


    cErrPrintf(TextWhite, "  %*s | ", lineNumberLength, "");
    for(int i = 0; i < length; i++) {
        if(i >= startPos && i < startPos + range->length) {
            cErrPutchar(color, '^');
        } else {
            cErrPutchar(color, ' ');
        }
    }

    cErrPutchar(TextRed, '\n');

    // line after error token
    printLine(range, lineNumberLength, 1, &start, &length);

    cErrPutchar(TextRed, '\n');
}

Error* errNew(ErrorLevel level) {
    CONTEXT(INFO, "Creating Error");
    Error* err = ArenaAlloc(sizeof(Error));

    err->level = level;
    err->severity = ERROR_ERROR;
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
    ARRAY_PUSH(*err, chunk, chunk);
}

void errAddSource(Error* err, SourceRange* loc) {
    CONTEXT(TRACE, "Adding source to error");
    ErrorChunk chunk;
    chunk.type = ERROR_CHUNK_SOURCE;
    chunk.as.source = *loc;
    ARRAY_PUSH(*err, chunk, chunk);
}

void errAddGraph(Error* err, Graph* graph) {
    CONTEXT(TRACE, "Adding graph to error");
    ErrorChunk chunk;
    chunk.type = ERROR_CHUNK_GRAPH;
    chunk.as.graph = *graph;
    ARRAY_PUSH(*err, chunk, chunk);
}

void errEmit(Error* err, struct Parser* parser) {
    CONTEXT(INFO, "Emitting created error");
    if(err->severity == ERROR_ERROR) {
        if(parser->panicMode) return;
        parser->hadError = true;

        if(err->level == ERROR_SYNTAX) {
            parser->panicMode = true;
        }
        setErrorState(parser);
    }

    ARRAY_PUSH(*parser, error, err);
}

void printErrors(Parser* parser) {
    int errors = 0;
    int warnings = 0;
    for(unsigned int i = 0; i < parser->errorCount; i++) {
        Error* err = parser->errors[i];
        if(err->level == ERROR_SYNTAX) {
            cErrPrintf(TextWhite, "Syntax ");
        } else if(err->level == ERROR_SEMANTIC) {
            cErrPrintf(TextWhite, "Semantic ");
        }
        if(err->severity == ERROR_ERROR) {
            cErrPrintf(TextWhite, "Error:\n");
            errors++;
        } else if(err->severity == ERROR_WARN) {
            cErrPrintf(TextWhite, "Warning:\n");
            warnings++;
        }

        TextColor color = TextWhite;
        for(unsigned int j = 0; j < err->chunkCount; j++) {
            ErrorChunk* chunk = &err->chunks[j];
            switch(chunk->type) {
                case ERROR_CHUNK_TEXT:
                    cErrPrintf(chunk->as.text.color, chunk->as.text.message);
                    cErrPrintf(TextWhite, "\n");
                    color = chunk->as.text.color;
                    break;
                case ERROR_CHUNK_SOURCE:
                    printMessage(&chunk->as.source, color);
                    break;
                case ERROR_CHUNK_GRAPH:
                    printGraph(&chunk->as.graph, cErrPrintf);
            }
        }
        cErrPrintf(TextWhite, "\n");
    }

    if(warnings > 0) {
        cErrPrintf(TextYellow, "Encountered %d warnings, continuing\n",
            warnings);
    }

    if(errors > 0) {
        cErrPrintf(TextRed, "Build Failed due to %d previous messages\n",
            errors);
    }
}
