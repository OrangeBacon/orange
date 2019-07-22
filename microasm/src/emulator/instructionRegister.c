#include "emulator/instructionRegister.h"
#include "shared/memory.h"

typedef struct instRegCtx {
    unsigned int bus;
    InstReg* reg;
} instRegCtx;

static void instRegSet(VMCore* core, void* vctx) {
    instRegCtx* ctx = vctx;
    uint16_t inst =  core->buss[ctx->bus].value;
    ctx->reg->value = inst;
    ctx->reg->opcode = (inst >> 9) & 0x7F;
    ctx->reg->arg1 =   (inst >> 6) & 0x7;
    ctx->reg->arg2 =   (inst >> 3) & 0x7;
    ctx->reg->arg3 =   (inst >> 0) & 0x7;
    ctx->reg->arg12 = (ctx->reg->arg1 << 0x3) + ctx->reg->arg2;
    ctx->reg->arg123 = (ctx->reg->arg1 << 0x6) + (ctx->reg->arg2 << 0x3) + ctx->reg->arg3;
}

unsigned int instRegInit(VMCore* core, InstReg* reg, unsigned int instBus) {
    instRegCtx* ctx = ArenaAlloc(sizeof(instRegCtx));
    ctx->bus = instBus;
    ctx->reg = reg;
    PUSH_ARRAY(Command, *core, command, instRegSet);
    PUSH_ARRAY(void*, *core, context, ctx);
    return core->commandCount - 1;
}