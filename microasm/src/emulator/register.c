#include "shared/memory.h"
#include "emulator/register.h"
#include "emulator/vmcore.h"
#include "assert.h"

void regInit(Register* reg) {
    reg->value = 0;
}

typedef struct regContext {
    unsigned int reg;
    unsigned int bus;
} regContext;

static void busToReg(VMCore* core, void* ctx) {
    core->registers[((regContext*)ctx)->reg].value = core->buss[((regContext*)ctx)->bus].value;
}

static void regToBus(VMCore* core, void* ctx) {
    core->buss[((regContext*)ctx)->bus].value = core->registers[((regContext*)ctx)->reg].value;
}

unsigned int regConnectBus(VMCore* core, unsigned int reg, unsigned int bus) {
    regContext* ctx = ArenaAlloc(sizeof(regContext));
    ctx->bus = bus;
    ctx->reg = reg;
    ADD_COMMAND(busToReg, ctx, &core->buss[bus]);
    CHANGES(&core->registers[reg]);

    ADD_COMMAND(regToBus, ctx, &core->registers[reg]);
    CHANGES(&core->buss[bus]);
    return core->commandCount - 2;
}

void regWriteInt(VMCore* core, unsigned int reg, uint16_t val) {
    core->registers[reg].value = val;
}

uint16_t regRead(VMCore* core, unsigned int reg) {
    return core->registers[reg].value;
}