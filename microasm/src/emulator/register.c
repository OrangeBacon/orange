#include "emulator/register.h"
#include "emulator/vmcore.h"
#include "assert.h"

void regInit(Register* reg) {
    ARRAY_ALLOC(Bus*, *reg, bus);
    reg->value = 0;
}

void regConnectBus(RegisterId* reg, BusId* bus) {
    assert(reg->core == bus->core);
    PUSH_ARRAY(Bus*, reg->core->registers[reg->id], bus, &bus->core->buss[bus->id]);
}

void regWriteInt(RegisterId* reg, uint16_t val) {
    reg->core->registers[reg->id].value = val;
}

uint16_t regRead(RegisterId* reg) {
    return reg->core->registers[reg->id].value;
}

void regSetBus(RegisterId* reg, BusId* bus) {
    assert(reg->core == bus->core);
    reg->core->buss[bus->id].value = reg->core->registers[reg->id].value;
}

void regReadBus(RegisterId* reg, BusId* bus) {
    reg->core->registers[reg->id].value = reg->core->buss[bus->id].value;
}