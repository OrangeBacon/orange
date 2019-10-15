#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "shared/memory.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/error.h"
#include "microcode/token.h"

void InitAST(AST* mcode) {
    ARRAY_ALLOC(const char*, *mcode, fileName);
    mcode->head.isValid = false;
    mcode->head.isPresent = false;
    ARRAY_ZERO(mcode->head, line);
    ARRAY_ALLOC(OpCode, *mcode, opcode);
    mcode->opsize.start = NULL;
    mcode->phase.start = NULL;
}
