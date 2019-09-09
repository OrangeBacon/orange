#include "emulator/compiletime/writeEmulator.h"

#include "emulator/compiletime/vmcoregen.h"

void writeEmulator(const char* filename, Parser* mcode) {
    VMCoreGen core;
    initCore(&core);

    unsigned int A = addRegister(&core, "A");
    unsigned int B = addRegister(&core, "B");
    unsigned int IP = addRegister(&core, "IP");
    unsigned int address = addBus(&core, "address");
    unsigned int data = addBus(&core, "data");
    unsigned int instBus = addBus(&core, "instBus");

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