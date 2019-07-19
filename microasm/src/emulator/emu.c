#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/register.h"
#include "emulator/bus.h"

void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    Register accumulator;
    regInit(&accumulator);
    Register R1;
    regInit(&R1);
    Bus dataBus;
    regConnectBus(&accumulator, &dataBus);
    regConnectBus(&R1, &dataBus);

    regWriteInt(&accumulator, 15);
    cOutPrintf(TextWhite, "%i\n", accumulator.value);
}