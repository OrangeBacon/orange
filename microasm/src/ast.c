#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "token.h"
#include "memory.h"

void InitMicrocode(Microcode* mcode) {
    ArenaInit(&mcode->arena);
}

void PrintMicrocode(Microcode* mcode) {
    printf("Microcode:\n");
    printf("  Header: %u", mcode->head.bitCount);
    for(int i = 0; i < mcode->head.bitCount; i++) {
        printf("\n    ");
        TokenPrint(&mcode->head.bits[i]);
    }
}

void FreeMicrocode(Microcode* mcode) {
    ArenaFree(&mcode->arena);
}