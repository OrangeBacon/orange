#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "token.h"

void PrintMicrocode(Microcode* mcode) {
    printf("Name: ");
    printf("%s", TokenNames[mcode->head->line->conditions[0]->name->type]);
    printf("\nValue: ");
    printf("%s", TokenNames[mcode->head->line->conditions[0]->value->type]);
}

void FreeMicrocode(Microcode* mcode) {
    free(mcode->head->line->conditions);
    free(mcode->head->line->bits);
    free(mcode->head->line);
    free(mcode->head);
}