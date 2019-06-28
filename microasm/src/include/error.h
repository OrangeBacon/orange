#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

struct Parser;
struct Token;

typedef struct Error {
    unsigned int id;
    struct Token token;
} Error;

void enableErrorPrint();
void disableErrorPrint();

bool vErrorAt(struct Parser* parser, unsigned int code, struct Token* token, const char* , va_list args);
bool errorAt(struct Parser* parser, unsigned int code, struct Token* token, const char* , ...);
bool vNoteAt(struct Parser* parser, struct Token* token, const char* message, va_list args);
bool noteAt(struct Parser* parser, struct Token* token, const char* message, ...);
bool vWarnAt(struct Parser* parser, unsigned int code, struct Token* token, const char* message, va_list args);
bool warnAt(struct Parser* parser, unsigned int code, struct Token* token, const char* message, ...);

bool vErrorAtCurrent(struct Parser* parser, unsigned int code, const char* message, va_list args);
bool errorAtCurrent(struct Parser* parser, unsigned int code, const char* message, ...);
bool warn(struct Parser* parser, unsigned int code, const char* message, ...);

#endif
