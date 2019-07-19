#include <stdio.h>
#include <string.h>
#include "shared/memory.h"
#include "microcode/token.h"

#define STRING_TOKEN(x) #x,

// the string names of each token type
const char* TokenNames[] = {
    FOREACH_TOKEN(STRING_TOKEN)
};

#undef STRING_TOKEN

// simple debug print 
void TokenPrint(Token* token) {
    printf("%.4i:%.4i %.17s: %.*s", token->line, token->column,
        TokenNames[token->type], token->length, TOKEN_GET(*token));
}

Token createStrToken(const char* str) {
    Token t;
    t.base = str;
    t.length = strlen(str);
    t.offset = 0;
    return t;
}

Token* createStrTokenPtr(const char* str) {
    Token* t = ArenaAlloc(sizeof(Token));
    t->base = str;
    t->length = strlen(str);
    t->offset = 0;
    return t;
}