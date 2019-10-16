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
        TokenNames[token->type], token->length, token->start);
}

Token createStrToken(const char* str) {
    Token t;
    t.start = str;
    t.length = strlen(str);
    return t;
}

Token* createStrTokenPtr(const char* str) {
    Token* t = ArenaAlloc(sizeof(Token));
    t->start = str;
    t->length = strlen(str);
    return t;
}

Token* createUIntTokenPtr(unsigned int num) {
    Token* t = ArenaAlloc(sizeof(Token));
    char* str = ArenaAlloc(sizeof(char) * 6);
    sprintf(str, "%u", num);
    t->start = str;
    t->length = strlen(str);
    t->data.value = num;
    return t;
}

// FNV-1a
uint32_t tokenHash(void* value) {
    Token* token = value;
    uint32_t hash = 2166126261u;

    for(int i = 0; i < token->length; i++) {
        hash ^= token->start[i];
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
    return strncmp(tokA->start, tokB->start, tokA->length) == 0;
}

const char* tokenAllocName(Token* tok) {
    char* ret = ArenaAlloc(sizeof(char) * tok->length + 1);
    strncpy(ret, tok->start, tok->length);
    ret[tok->length] = '\0';
    return ret;
}