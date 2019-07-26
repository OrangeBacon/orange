#include "shared/memory.h"
#include "emulator/register.h"
#include "emulator/vmcore.h"
#include "shared/platform.h"
#include <stdlib.h>

// initialise a new register
void regInit(Register* reg) {
    reg->value = 0;
}

// infomation for microcode commands
typedef struct regContext {
    unsigned int reg;
    unsigned int bus;
} regContext;

// store the bus's data in the register
static void busToReg(VMCore* core, void* ctx) {
    // cannot store invalid data
    if(!core->buss[((regContext*)ctx)->bus].isValid) {
        cErrPrintf(TextRed, "invalid bus access - regwt");
    }
    core->registers[((regContext*)ctx)->reg].value = core->buss[((regContext*)ctx)->bus].value;
}

// write the contents of the register to the bus
static void regToBus(VMCore* core, void* ctx) {
    core->buss[((regContext*)ctx)->bus].value = core->registers[((regContext*)ctx)->reg].value;
    core->buss[((regContext*)ctx)->bus].isValid = true;
}

// add commands that move data between a bus and a register
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

// debug set
void regWriteInt(VMCore* core, unsigned int reg, uint16_t val) {
    core->registers[reg].value = val;
}

// debug read
uint16_t regRead(VMCore* core, unsigned int reg) {
    return core->registers[reg].value;
}