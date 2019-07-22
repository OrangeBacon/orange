#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/emulator.h"
#include "emulator/vmcore.h"
#include "emulator/register.h"

void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    VMCore core;
    vmcoreInit(&core);

    unsigned int accumulator = createRegister(&core);
    unsigned int R1 = createRegister(&core);
    unsigned int dataBus = createBus(&core);
    
    regConnectBus(&core, accumulator, dataBus);
    regConnectBus(&core, R1, dataBus);

    regWriteInt(&core, accumulator, 15);
    regSetBus(&core, accumulator, dataBus);
    regWriteInt(&core, accumulator, 10);
    regReadBus(&core, R1, dataBus);

    cOutPrintf(TextWhite, "%i\n", regRead(&core, R1));
}