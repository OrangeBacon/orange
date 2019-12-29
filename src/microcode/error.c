#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shared/memory.h"
#include "shared/platform.h"
#include "microcode/parser.h"
#include "microcode/error.h"
#include "shared/log.h"

// TODO: properly implement errors at end of file

static void printLine(Parser* parser, int line, int* start, int* length, int lineNumberLength) {
    if(getLine(parser->scanner->base, line, start, length)) {
        cErrPrintf(TextWhite, "  %*i | %.*s\n", lineNumberLength, line, *length, parser->scanner->base + *start);
    }
}

// print a message about a token to stderr
void printMessage(Parser* parser, SourceRange* range, const char* name, unsigned int code, TextColor color,
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
}

typedef struct ErrorList {
    DEFINE_ARRAY(Error*, error);
} ErrorList;
static ErrorList possibleErrors = {0};

static void newErr(Error* error, ErrorLevel level, const char* message) {
    CONTEXT(TRACE, "Creating new error definition");
    if(possibleErrors.errors == NULL) {
        ARRAY_ALLOC(Error*, possibleErrors, error);
    }
    PUSH_ARRAY(Error*, possibleErrors, error, error);

    error->id = possibleErrors.errorCount;
    error->level = level;
    error->message = message;
    TRACE("Allocating error note array");
    ARRAY_ALLOC(const char*, *error, note);
}

void newErrAt(Error* error, ErrorLevel level, const char* message) {
    newErr(error, level, message);
    error->location = EL_ASK;
}

void newErrCurrent(Error* error, ErrorLevel level, const char* message) {
    newErr(error, level, message);
    error->location = EL_CURRENT;
}

void newErrPrevious(Error* error, ErrorLevel level, const char* message) {
    newErr(error, level, message);
    error->location = EL_PREVIOUS;
}

void newErrEnd(Error* error, ErrorLevel level, const char* message) {
    newErr(error, level, message);
    error->location = EL_END;
}

void newErrConsume(Error* error, ErrorLevel level,
    MicrocodeTokenType type, const char* message) {

    newErr(error, level, message);
    error->consumeType = type;
    error->location = EL_CURRENT;
}

void newErrNoteAt(Error* error, const char* message) {
    PUSH_ARRAY(const char*, *error, note, message);
}

static char* getMessagePrint(const char* message, va_list args) {
    va_list temp;

    va_copy(temp, args);
    size_t len = vsnprintf(NULL, 0, message, temp) + 1;
    va_end(temp);

    char* buf = ArenaAlloc(len * sizeof(char));
    vsprintf(buf, message, args);

    return buf;
}

void vError(Parser* parser, Error* error, va_list args) {
    CONTEXT(INFO, "Emitting error");
    if(parser->panicMode) return;
    parser->hadError = true;

    EmittedError eErr = {0};
    eErr.atEnd = false;
    eErr.parser = parser;
    eErr.error = error;

    switch(error->location) {
        case EL_ASK:
            eErr.range = *va_arg(args, SourceRange*);
            break;
        case EL_CURRENT:
            eErr.range = parser->current.range;
            break;
        case EL_PREVIOUS:
            eErr.range = parser->previous.range;
            break;
        case EL_END:
            eErr.atEnd = true;
            break;
    }

    switch(error->level) {
        case ERROR_SEMANTIC:
            eErr.name = "Semantic Error";
            eErr.color = TextRed;
            break;
        case ERROR_SYNTAX:
            eErr.name = "Syntax Error";
            eErr.color = TextRed;
            parser->panicMode = true;
            break;
    }

    eErr.message = getMessagePrint(error->message, args);

    if(error->noteCount > 0) {
        ARRAY_ALLOC(EmittedErrorNoteData, eErr, noteData);
    }

    for(unsigned int i = 0; i < error->noteCount; i++) {
        EmittedErrorNoteData data = {0};
        data.token = *va_arg(args, SourceRange*);
        data.message = getMessagePrint(error->notes[i], args);
        PUSH_ARRAY(EmittedErrorNoteData, eErr, noteData, data);
    }

    PUSH_ARRAY(EmittedError, *parser, error, eErr);
}

void error(Parser* parser, Error* error, ...) {
    va_list args;
    va_start(args, error);
    vError(parser, error, args);
    va_end(args);
}

void printErrors(Parser* parser) {
    CONTEXT(DEBUG, "Printing all errors");
    for(unsigned int i = 0; i < parser->errorCount; i++) {
        EmittedError* err = &parser->errors[i];
        printMessage(err->parser, &err->range, err->name, err->error->id, err->color, err->message);

        for(unsigned int j = 0; j < err->noteDataCount; j++) {
            EmittedErrorNoteData* data = &err->noteDatas[j];
            printMessage(err->parser, &data->token, "Note", 0, TextBlue, data->message);
        }
    }
}
