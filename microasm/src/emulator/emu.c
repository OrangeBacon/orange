#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/emulator.h"
#include "emulator/vmcore.h"
#include "emulator/register.h"
#include "emulator/memory64k.h"

void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    VMCore core;
    Memory64k mem = {0};

    vmcoreInit(&core);

    unsigned int A = createRegister(&core);
    unsigned int B = createRegister(&core);
    unsigned int addressBus = createBus(&core);
    unsigned int dataBus = createBus(&core);

    unsigned int memAccess = memoryInit(&mem, &core, addressBus, dataBus);
    unsigned int dataToA = regConnectBus(&core, A, dataBus);
    regConnectBus(&core, B, dataBus);
    regConnectBus(&core, A, addressBus);
    unsigned int addrToB = regConnectBus(&core, B, addressBus);

    regWriteInt(&core, A, 15);
    regWriteInt(&core, B, 11);
    coreCall(&core, dataToA + 1);
    coreCall(&core, addrToB + 1);
    coreCall(&core, memAccess + 1);
    coreCall(&core, memAccess);
    coreCall(&core, dataToA);

    cOutPrintf(TextWhite, "%i\n", mem.value[11]);
}