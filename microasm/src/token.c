#include <stdio.h>
#include "token.h"

const char* TokenNames[] = {
    FOREACH_TOKEN(STRING_TOKEN)
};

void TokenPrint(Token* token) {
    printf("%.4i:%.4i %.17s: %.*s", token->line, token->column,
        TokenNames[token->type], token->length, token->start);
}