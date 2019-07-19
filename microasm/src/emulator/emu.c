#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/emulator.h"
#include "emulator/vmcore.h"
#include "emulator/register.h"

void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    VMCore core;
    vmcoreInit(&core);

    RegisterId accumulator = createRegister(&core);
    RegisterId R1 = createRegister(&core);
    BusId dataBus = createBus(&core);
    
    regConnectBus(&accumulator, &dataBus);
    regConnectBus(&R1, &dataBus);

    regWriteInt(&accumulator, 15);
    regSetBus(&accumulator, &dataBus);
    regWriteInt(&accumulator, 10);
    regReadBus(&R1, &dataBus);

    cOutPrintf(TextWhite, "%i\n", regRead(&R1));
}