#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include "shared/platform.h"

struct Parser;
struct Token;

typedef enum ErrorLevel {
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
} ErrorLevel;

typedef enum ErrorLocation {
    EL_ASK,
    EL_CURRENT,
    EL_PREVIOUS,
    EL_END
} ErrorLocation;

typedef struct Error {
    unsigned int id;
    ErrorLevel level;
    const char* message;
    ErrorLocation location;
    MicrocodeTokenType consumeType;
    DEFINE_ARRAY(const char*, note);
} Error;

typedef struct EmittedErrorNoteData {
    Token token;
    const char* message;
} EmittedErrorNoteData;

typedef struct EmittedError {
    Error* error;
    Token token;
    bool atEnd;
    const char* message;
    const char* name;
    TextColor color;
    struct Parser* parser;
    DEFINE_ARRAY(EmittedErrorNoteData, noteData);
} EmittedError;

void newErrAt(Error* error, ErrorLevel level, const char* message);
void newErrCurrent(Error* error, ErrorLevel level, const char* message);
void newErrPrevious(Error* error, ErrorLevel level, const char* message);
void newErrEnd(Error* error, ErrorLevel level, const char* message);
void newErrConsume(Error* error, ErrorLevel level,
    MicrocodeTokenType type, const char* message);
void newErrNoteAt(Error* error, const char* message);

void vError(struct Parser* parser, Error* error, va_list args);
void error(struct Parser* parser, Error* error, ...);

void printErrors(struct Parser* parser);

#endif
