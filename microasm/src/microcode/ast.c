#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "shared/memory.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/error.h"
#include "microcode/token.h"

void InitAST(AST* mcode, const char* fileName) {
    mcode->fileName = fileName;
    mcode->head.isValid = false;
    mcode->inp.isValid = false;
    ARRAY_ZERO(mcode->head, line);
    ARRAY_ZERO(mcode->inp, value);
    ARRAY_ALLOC(OpCode, *mcode, opcode);
    ARRAY_ALLOC(Error, *mcode, error);
#ifdef debug
    ARRAY_ALLOC(Error, *mcode, expectedError);
#endif
}