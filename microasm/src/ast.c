#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "token.h"
#include "memory.h"

void InitMicrocode(Microcode* mcode, const char* fileName) {
    ArenaInit(&mcode->arena);
    mcode->hasError = false;
    mcode->fileName = fileName;
}

void PrintMicrocode(Microcode* mcode) {
    printf("Microcode:\n");
    printf("  Has Error: %s\n", mcode->hasError ? "true" : "false");
    printf("  File Name: %s\n", mcode->fileName);
    printf("  Header: %u", mcode->head.bitCount);
    for(int i = 0; i < mcode->head.bitCount; i++) {
        printf("\n    ");
        TokenPrint(&mcode->head.bits[i]);
    }
    printf("\n");
}

void FreeMicrocode(Microcode* mcode) {
    ArenaFree(&mcode->arena);
}