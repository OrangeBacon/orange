#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/emulator.h"
#include "emulator/vmcore.h"
#include "emulator/register.h"
#include "emulator/memory64k.h"
#include "emulator/instructionRegister.h"

void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    VMCore core;
    Memory64k mem = {0};
    InstReg instreg;

    vmcoreInit(&core);

    unsigned int A = createRegister(&core);
    unsigned int B = createRegister(&core);
    unsigned int addressBus = createBus(&core);
    unsigned int dataBus = createBus(&core);

    unsigned int memAccess = memoryInit(&mem, &core, addressBus, dataBus);
    unsigned int setIReg = instRegInit(&core, &instreg, dataBus);
    unsigned int dataToA = regConnectBus(&core, A, dataBus);
    regConnectBus(&core, B, dataBus);
    regConnectBus(&core, A, addressBus);
    unsigned int addrToB = regConnectBus(&core, B, addressBus);

    regWriteInt(&core, A, 0x3111);
    regWriteInt(&core, B, 11);
    coreCall(&core, dataToA + 1);
    coreCall(&core, addrToB + 1);
    coreCall(&core, memAccess + 1);
    coreCall(&core, memAccess);
    coreCall(&core, dataToA);
    coreCall(&core, dataToA + 1);
    coreCall(&core, setIReg);

    cOutPrintf(TextWhite, "%i\n", mem.value[11]);
}