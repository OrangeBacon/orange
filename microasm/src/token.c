#include <stdio.h>
#include "token.h"

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