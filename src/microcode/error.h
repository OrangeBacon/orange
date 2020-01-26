#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include "shared/platform.h"
#include "microcode/token.h"

typedef enum ErrorLevel {
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
} ErrorLevel;

typedef struct ErrorChunk {
    enum {
        ERROR_CHUNK_TEXT,
        ERROR_CHUNK_SOURCE
    } type;

    union {
        struct {
            const char* message;
            TextColor color;
        } text;
        SourceRange source;
    } as;
} ErrorChunk;

typedef struct Error {
    ErrorLevel level;
    DEFINE_ARRAY(ErrorChunk, chunk);
} Error;

Error* errNew(ErrorLevel level);
void errAddText(Error* err, TextColor color, const char* text, ...);
void vErrAddText(Error* err, TextColor color, const char* text, va_list args);
void errAddSource(Error* err, SourceRange* loc);
void errEmit(Error* err, struct Parser* parser);

void printErrors(struct Parser* parser);

#endif
