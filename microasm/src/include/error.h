#ifndef ERROR_H
#define ERROR_H

#include "parser.h"
#include "platform.h"

bool vErrorAt(Parser* parser, Token* token, const char* , va_list args);
bool errorAt(Parser* parser, Token* token, const char* , ...);
bool vNoteAt(Parser* parser, Token* token, const char* message, va_list args);
bool noteAt(Parser* parser, Token* token, const char* message, ...);
bool vWarnAt(Parser* parser, Token* token, const char* message, va_list args);
bool warnAt(Parser* parser, Token* token, const char* message, ...);

bool vErrorAtCurrent(Parser* parser, const char* message, va_list args);
bool errorAtCurrent(Parser* parser, const char* message, ...);
bool warn(Parser* parser, const char* message, ...);

#endif