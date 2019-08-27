#include "emulator/runtime/instructionRegister.h"
#include "shared/memory.h"
#include "shared/platform.h"
#include <stdlib.h>

// register to store the opcode and arguments of the currently
// executing instruction

// context for the microcode command
typedef struct instRegCtx {
    // which bus to read from
    unsigned int bus;
    // which register to set the values in
    instructionRegister* reg;
} instRegCtx;

// set the values in the intruction register
static void instRegSet(VMCore* core, void* vctx) {
    // cast context to correct type
    instRegCtx* ctx = vctx;

    // should not read from invalid busses
    // propper error handling todo
    if(!core->buss[ctx->bus].isValid) {
        cErrPrintf(TextRed, "Invalid bus read - ireg");
        exit(1);
    }

    uint16_t inst =  core->buss[ctx->bus].value;
    ctx->reg->value = inst;
    ctx->reg->opcode = (inst >> 9) & 0x7F; // first 7 bits
    ctx->reg->arg1 =   (inst >> 6) & 0x7;  // next 3 bits
    ctx->reg->arg2 =   (inst >> 3) & 0x7;  // next 3 bits
    ctx->reg->arg3 =   (inst >> 0) & 0x7;  // next 3 bits
    ctx->reg->arg12 = (ctx->reg->arg1 << 0x7) + ctx->reg->arg2;  // bits 8-13
    ctx->reg->arg123 = (ctx->reg->arg1 << 0x7) + 
        (ctx->reg->arg2 << 0x7) + ctx->reg->arg3; // bits 8-16
}

unsigned int instRegInit(VMCore* core, instructionRegister* reg, unsigned int instBus) {
    // context needs to live for life of vm
    instRegCtx* ctx = ArenaAlloc(sizeof(instRegCtx));
    ctx->bus = instBus;
    ctx->reg = reg;

    // set up command
    ADD_COMMAND(instRegSet, ctx, &core->buss[instBus]);
    CHANGES(reg);

    // return the command's id 
    return core->commandCount - 1;
}