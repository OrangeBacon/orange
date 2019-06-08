#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "token.h"
#include "memory.h"

void InitMicrocode(Microcode* mcode, const char* fileName) {
    mcode->hasError = false;
    mcode->fileName = fileName;
    ARRAY_ZERO(mcode->head, bit);
    ARRAY_ZERO(mcode->inp, value);
    ARRAY_ZERO(mcode->out, value);
    mcode->out.width = 0;
}

void PrintMicrocode(Microcode* mcode) {
    printf("Microcode:\n");
    printf("  Has Error: %s\n", mcode->hasError ? "true" : "false");
    printf("  File Name: %s\n", mcode->fileName);
    printf("  Header: %u", mcode->head.bitCount);
    for(unsigned int i = 0; i < mcode->head.bitCount; i++) {
        printf("\n    ");
        TokenPrint(&mcode->head.bits[i]);
    }
    printf("\n");
    printf("  Input: %u", mcode->inp.valueCount);
    for(unsigned int i = 0; i < mcode->inp.valueCount; i++) {
        printf("\n    ");
        TokenPrint(&mcode->inp.values[i].name);
        printf(" = %i", mcode->inp.values[i].value);
    }
    printf("\n");
    printf("  Output(%u): %u", mcode->out.width, mcode->out.valueCount);
    for(unsigned int i = 0; i < mcode->out.valueCount; i++) {
        printf("\n    ");
        printf("%i = ", mcode->out.values[i].id);
        TokenPrint(&mcode->out.values[i].name);
    }
    printf("\n");
}