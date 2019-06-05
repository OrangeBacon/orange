#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "token.h"
#include "memory.h"

void InitMicrocode(Microcode* mcode) {
    ArenaInit(&mcode->arena);
}

void PrintMicrocode(Microcode* mcode) {
    printf("Name: ");
    printf("%s", TokenNames[mcode->head.line.conditions[0].name.type]);
    printf("\nValue: ");
    printf("%s", TokenNames[mcode->head.line.conditions[0].value.type]);
}

void FreeMicrocode(Microcode* mcode) {
    ArenaFree(&mcode->arena);
}