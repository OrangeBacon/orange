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
    
    regConnectBus(&core, &accumulator, &dataBus);
    regConnectBus(&core, &R1, &dataBus);

    regWriteInt(&accumulator, 15);
}