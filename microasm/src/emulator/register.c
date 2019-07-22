#include "shared/memory.h"
#include "emulator/register.h"
#include "emulator/vmcore.h"
#include "assert.h"

void regInit(Register* reg) {
    ARRAY_ALLOC(Bus*, *reg, bus);
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
    PUSH_ARRAY(Bus*, core->registers[reg], bus, &core->buss[bus]);
    regContext* ctx = ArenaAlloc(sizeof(regContext));
    ctx->bus = bus;
    ctx->reg = reg;
    PUSH_ARRAY(Command, *core, command, busToReg);
    PUSH_ARRAY(void*, *core, context, ctx);
    PUSH_ARRAY(Command, *core, command, regToBus);
    PUSH_ARRAY(void*, *core, context, ctx);
    return core->commandCount - 2;
}

void regWriteInt(VMCore* core, unsigned int reg, uint16_t val) {
    core->registers[reg].value = val;
}

uint16_t regRead(VMCore* core, unsigned int reg) {
    return core->registers[reg].value;
}