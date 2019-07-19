#include "emulator/register.h"
#include "emulator/vmcore.h"

void regConnectBus(VMCore* core, RegisterId* reg, BusId* bus) {
    (void)core;
    (void)reg;
    (void)bus;
}

void regWriteInt(RegisterId* reg, uint16_t val) {
    (void)reg;
    (void)val;
}