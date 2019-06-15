#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "token.h"
#include "ast.h"
#include "error.h"
#include "token.h"
#include "memory.h"

void InitMicrocode(Microcode* mcode, const char* fileName) {
    mcode->hasError = false;
    mcode->fileName = fileName;
    ARRAY_ZERO(mcode->head, bit);
    ARRAY_ZERO(mcode->inp, value);
    ARRAY_ZERO(mcode->out, value);
    ARRAY_ALLOC(OpCode, *mcode, opcode);
    ARRAY_ALLOC(Error, *mcode, error);
#ifdef debug
    ARRAY_ALLOC(Error, *mcode, expectedError);
#endif
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
        printf(" = %i", mcode->inp.values[i].value.data.value);
    }
    printf("\n");
    printf("  Output(%u): %u", mcode->out.width.data.value, mcode->out.valueCount);
    for(unsigned int i = 0; i < mcode->out.valueCount; i++) {
        printf("\n    ");
        printf("%i = ", mcode->out.values[i].id.data.value);
        TokenPrint(&mcode->out.values[i].name);
    }
    printf("\n");
    printf("  OpCodes: %u", mcode->opcodeCount);
    for(unsigned int i = 0; i < mcode->opcodeCount; i++) {
        OpCode* code = &mcode->opcodes[i];
        printf("\n    OpCode: ");
        TokenPrint(&code->name);
        printf(" = %u", code->id.data.value);
        printf("\n      Parameters: %u", code->parameterCount);
        for(unsigned int j = 0; j < code->parameterCount; j++) {
            printf("\n        ");
            TokenPrint(&code->parameters[j]);
        }
        printf("\n      Lines: %u", code->lineCount);
        for(unsigned int j = 0; j < code->lineCount; j++) {
            Line* line = code->lines[j];
            printf("\n        Line %u:", j);
            if(line->anyCondition) {
                printf("\n          Conditions: any");
            } else {
                printf("\n          Conditions: %u", line->conditionCount);
                for(unsigned int k = 0; k < line->conditionCount; k++) {
                    printf("\n            ");
                    TokenPrint(&line->conditions[k].name);
                    printf(" = %u", line->conditions[k].value.data.value);
                }
            }
            printf("\n          Bits: %u", line->bitCount);
            for(unsigned int k = 0; k < line->bitCount; k++) {
                printf("\n            ");
                TokenPrint(&line->bits[k]);
            }
        }
    }
    printf("\n");
}