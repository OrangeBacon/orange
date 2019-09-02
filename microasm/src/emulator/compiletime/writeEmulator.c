#include "emulator/compiletime/writeEmulator.h"

#include "emulator/compiletime/vmcoregen.h"

void writeEmulator(const char* filename, Parser* mcode) {
    (void)mcode;

    VMCoreGen core;
    initCore(&core);

    Register A = addRegister(&core, "A");
    Register B = addRegister(&core, "B");
    Register IP = addRegister(&core, "IP");
    Bus address = addBus(&core, "address");
    Bus data = addBus(&core, "data");
    Bus instBus = addBus(&core, "instBus");

    addBusRegisterConnection(&core, data, A, 0);
    addBusRegisterConnection(&core, data, B, 0);
    addBusRegisterConnection(&core, address, A, 0);
    addBusRegisterConnection(&core, address, B, 0);
    addBusRegisterConnection(&core, address, IP, 1);

    addInstructionRegister(&core, instBus);
    Memory mem = addMemory64k(&core, address, data);
    addMemoryBusOutput(&core, &mem, instBus);

    addHaltInstruction(&core);

    addCoreLoop(&core, mcode);

    writeCore(&core, filename);
}