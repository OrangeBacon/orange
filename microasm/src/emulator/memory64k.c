#include "emulator/memory64k.h"
#include "shared/memory.h"
#include "shared/platform.h"
#include <stdlib.h>

// context for microcode commands
typedef struct memCtx {
    unsigned int addressBus; 
    unsigned int dataBus;
    Memory64k* mem;
} memCtx;

// read a value from memory
static void memoryRead(VMCore* core, void* vctx) {
    // cast context
    memCtx* ctx = vctx;

    // cannot read from unassigned bus
    if(!core->buss[ctx->addressBus].isValid) {
        cErrPrintf(TextRed, "invalid bus read - memrd");
        exit(0);
    }

    // set bus values
    core->buss[ctx->dataBus].value = ctx->mem->value[core->buss[ctx->addressBus].value];

    // bus has been written to
    core->buss[ctx->dataBus].isValid = true;
}

// write to memory
static void memoryWrite(VMCore* core, void* vctx) {
    // cast context
    memCtx* ctx = vctx;

    // check both busses have been written to
    if(!core->buss[ctx->addressBus].isValid || !core->buss[ctx->dataBus].isValid) {
        cErrPrintf(TextRed, "invalid bus read - memwt");
        exit(0);
    }

    // set memory value
    ctx->mem->value[core->buss[ctx->addressBus].value] = core->buss[ctx->dataBus].value;
}

// set up microcode commands for a memory block
unsigned int memoryInit(Memory64k* mem, VMCore* core, unsigned int addressBus, unsigned int dataBus) {
    // allocate so lives for length of vm
    memCtx* ctx = ArenaAlloc(sizeof(memCtx));
    ctx->addressBus = addressBus;
    ctx->dataBus = dataBus;
    ctx->mem = mem;

    ADD_COMMAND(memoryRead, ctx, &core->buss[addressBus], mem);
    CHANGES(&core->buss[dataBus]);

    ADD_COMMAND(memoryWrite, ctx, &core->buss[addressBus], &core->buss[dataBus]);
    CHANGES(&mem);

    // return id of memory read, returned value + 1 = memory write
    return core->commandCount - 2;
}