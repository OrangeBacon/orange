#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "shared/memory.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/error.h"
#include "microcode/token.h"

void InitAST(AST* mcode, const char* fileName) {
    mcode->hasError = false;
    mcode->fileName = fileName;
    mcode->head.isValid = false;
    mcode->inp.isValid = false;
    mcode->out.isValid = false;
    ARRAY_ZERO(mcode->head, line);
    ARRAY_ZERO(mcode->inp, value);
    ARRAY_ZERO(mcode->out, value);
    ARRAY_ALLOC(OpCode, *mcode, opcode);
    ARRAY_ALLOC(Error, *mcode, error);
#ifdef debug
    ARRAY_ALLOC(Error, *mcode, expectedError);
#endif
}

void initMicrocode(Microcode* mcode) {
    ARRAY_ALLOC(NumericOpcode, *mcode, opcode);
}

void PrintAST(AST* mcode) {
    printf("Microcode:\n");
    printf("  Has Error: %s\n", mcode->hasError ? "true" : "false");
    printf("  File Name: %s\n", mcode->fileName);
    printf("  Header: %u", mcode->head.lines->dataCount);
    for(unsigned int i = 0; i < mcode->head.lines->dataCount; i++) {
        printf("\n    ");
        TokenPrint(&mcode->head.lines->datas[i]);
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
            printf("\n          Bits: %u", line->bits.dataCount);
            for(unsigned int k = 0; k < line->bits.dataCount; k++) {
                printf("\n            ");
                TokenPrint(&line->bits.datas[k]);
            }
        }
    }
    printf("\n");
}