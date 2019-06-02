#include <stdio.h>
#include "token.h"

const char* TokenNames[] = {
    FOREACH_TOKEN(STRING_TOKEN)
};

void TokenPrint(Token* token) {
    printf("Token(%s)", TokenNames[token->type]);
}