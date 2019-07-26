#include "shared/platform.h"
#include "emulator/emu.h"
#include "emulator/emulator.h"
#include "emulator/vmcore.h"
#include "emulator/register.h"
#include "emulator/memory64k.h"
#include "emulator/instructionRegister.h"

// This file contains a simple test for the rest of the emulator and will
// be re-written or removed when all of the emulator works, with regard to
// loading microcdes and executable programs.

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
    coreCallLine(&core, 3,
    dataToA + 1,
    addrToB + 1,
    memAccess + 1);
    coreCallLine(&core, 3, memAccess, addrToB + 1, dataToA);
    coreCallLine(&core, 2,
    dataToA + 1,
    setIReg);

    cOutPrintf(TextWhite, "%i\n", instreg.opcode);
}