#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
#include "shared/memory.h"
#include "emulator/bus.h"

typedef struct Register {
    uint16_t value;
    DEFINE_ARRAY(Bus*, bus);
} Register;

void regInit(Register* reg);
void regConnectBus(Register* reg, Bus* bus);
void regWriteInt(Register* reg, uint16_t val);

#endif