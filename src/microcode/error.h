#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include "shared/platform.h"
#include "shared/graph.h"
#include "microcode/token.h"

typedef enum ErrorLevel {
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
} ErrorLevel;

typedef struct ErrorChunk {
    enum {
        ERROR_CHUNK_TEXT,
        ERROR_CHUNK_SOURCE,
        ERROR_CHUNK_GRAPH
    } type;

    union {
        struct {
            const char* message;
            TextColor color;
        } text;
        SourceRange source;
        Graph graph;
    } as;
} ErrorChunk;

typedef struct Error {
    ErrorLevel level;
    ARRAY_DEFINE(ErrorChunk, chunk);

    enum {
        ERROR_ERROR,
        ERROR_WARN
    } severity;
} Error;

Error* errNew(ErrorLevel level);
void errAddText(Error* err, TextColor color, const char* text, ...);
void vErrAddText(Error* err, TextColor color, const char* text, va_list args);
void errAddSource(Error* err, SourceRange* loc);
void errAddGraph(Error* err, Graph* graph);
void errEmit(Error* err, struct Parser* parser);

void printErrors(struct Parser* parser);

#endif
