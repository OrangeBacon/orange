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
    printf("%.4i:%.4i %.17s: %.*s", token->range.line, token->range.column,
        TokenNames[token->type], token->range.length, token->range.tokenStart);
}

Token createStrToken(const char* str) {
    Token t;
    t.range.tokenStart = str;
    t.range.length = strlen(str);
    t.data.string = str;
    return t;
}

Token* createStrTokenPtr(const char* str) {
    Token* t = ArenaAlloc(sizeof(Token));
    t->range.tokenStart = str;
    t->range.length = strlen(str);
    return t;
}

Token* createUIntTokenPtr(unsigned int num) {
    Token* t = ArenaAlloc(sizeof(Token));
    char* str = ArenaAlloc(sizeof(char) * 6);
    sprintf(str, "%u", num);
    t->range.tokenStart = str;
    t->range.length = strlen(str);
    t->data.value = num;
    return t;
}

const char* tokenAllocName(Token* tok) {
    char* ret = ArenaAlloc(sizeof(char) * tok->range.length + 1);
    strncpy(ret, tok->range.tokenStart, tok->range.length);
    ret[tok->range.length] = '\0';
    return ret;
}