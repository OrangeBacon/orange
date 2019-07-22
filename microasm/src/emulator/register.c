#include "emulator/register.h"
#include "emulator/vmcore.h"
#include "assert.h"

void regInit(Register* reg) {
    ARRAY_ALLOC(Bus*, *reg, bus);
    reg->value = 0;
}

void regConnectBus(VMCore* core, unsigned int reg, unsigned int bus) {
    PUSH_ARRAY(Bus*, core->registers[reg], bus, &core->buss[bus]);
}

void regWriteInt(VMCore* core, unsigned int reg, uint16_t val) {
    core->registers[reg].value = val;
}

uint16_t regRead(VMCore* core, unsigned int reg) {
    return core->registers[reg].value;
}

void regSetBus(VMCore* core, unsigned int reg, unsigned int bus) {
    core->buss[bus].value = core->registers[reg].value;
}

void regReadBus(VMCore* core, unsigned int reg, unsigned int bus) {
    core->registers[reg].value = core->buss[bus].value;
}