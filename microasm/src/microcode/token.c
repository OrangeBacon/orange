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

Token* createUIntTokenPtr(unsigned int num) {
    Token* t = ArenaAlloc(sizeof(Token));
    char* str = ArenaAlloc(sizeof(char) * 6);
    sprintf(str, "%u", num);
    t->base = str;
    t->length = strlen(str);
    t->offset = 0;
    t->data.value = num;
    return t;
}

// FNV-1a
uint32_t tokenHash(void* value) {
    Token* token = value;
    uint32_t hash = 2166126261u;

    for(int i = 0; i < token->length; i++) {
        hash ^= TOKEN_GET(*token)[i];
        hash *= 16777619;
    }

    return hash;
}

bool tokenCmp(void* a, void* b) {
    Token* tokA = a;
    Token* tokB = b;

    if(tokA->length != tokB->length) {
        return false;
    }
    return strncmp(TOKEN_GET(*tokA), TOKEN_GET(*tokB), tokA->length) == 0;
}
