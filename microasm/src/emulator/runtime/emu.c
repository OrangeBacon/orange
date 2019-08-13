#include "shared/platform.h"
#include "emulator/runtime/emu.h"
#include "emulator/runtime/emulator.h"
#include "emulator/runtime/vmcore.h"
#include "emulator/runtime/register.h"
#include "emulator/runtime/memory64k.h"
#include "emulator/runtime/instructionRegister.h"

// This file contains a simple test for the rest of the emulator and will
// be re-written or removed when all of the emulator works, with regard to
// loading microcdes and executable programs.
/*
void emulator() {
    cOutPrintf(TextGreen, "Emulation\n");

    VMCore core;
    Memory64k mem = {0};
    InstReg instreg;

    vmcoreInit(&core);

    unsigned int A = createRegister(&core, "A");
    unsigned int B = createRegister(&core, "B");
    unsigned int addressBus = createBus(&core, "address");
    unsigned int dataBus = createBus(&core, "data");

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

    vmcoreFree(&core);
}*/