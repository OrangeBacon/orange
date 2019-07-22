#include "emulator/memory64k.h"
#include "shared/memory.h"

typedef struct memCtx {
    unsigned int addressBus; 
    unsigned int dataBus;
    Memory64k* mem;
} memCtx;

static void memoryRead(VMCore* core, void* vctx) {
    memCtx* ctx = vctx;
    core->buss[ctx->dataBus].value = ctx->mem->value[core->buss[ctx->addressBus].value];
}

static void memoryWrite(VMCore* core, void* vctx) {
    memCtx* ctx = vctx;
    ctx->mem->value[core->buss[ctx->addressBus].value] = core->buss[ctx->dataBus].value;
}

unsigned int memoryInit(Memory64k* mem, VMCore* core, unsigned int addressBus, unsigned int dataBus) {
    memCtx* ctx = ArenaAlloc(sizeof(memCtx));
    ctx->addressBus = addressBus;
    ctx->dataBus = dataBus;
    ctx->mem = mem;
    PUSH_ARRAY(Command, *core, command, memoryRead);
    PUSH_ARRAY(void*, *core, context, ctx);
    PUSH_ARRAY(Command, *core, command, memoryWrite);
    PUSH_ARRAY(void*, *core, context, ctx);
    return core->commandCount - 2;
}