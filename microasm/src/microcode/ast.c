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
    mcode->inp.isValid = false;
    mcode->inp.isPresent = false;
    ARRAY_ZERO(mcode->head, line);
    ARRAY_ZERO(mcode->inp, value);
    ARRAY_ALLOC(OpCode, *mcode, opcode);
    ARRAY_ALLOC(Error, *mcode, error);
}
