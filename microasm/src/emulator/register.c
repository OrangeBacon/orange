#include "emulator/register.h"

void regInit(Register* reg) {
    ARRAY_ALLOC(Bus*, *reg, bus);
}

void regConnectBus(Register* reg, Bus* bus) {
    PUSH_ARRAY(Bus*, *reg, bus, bus);
}

void regWriteInt(Register* reg, uint16_t val) {
    reg->value = val;
}