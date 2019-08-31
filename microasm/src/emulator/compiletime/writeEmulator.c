#include "emulator/compiletime/writeEmulator.h"

#include "emulator/compiletime/vmcoregen.h"

void writeEmulator(const char* filename, Parser* mcode) {
    (void)mcode;

    VMCoreGen core;
    initCore(&core);

    Register A = addRegister(&core, "A");
    Register B = addRegister(&core, "B");
    Bus address = addBus(&core, "address");
    Bus data = addBus(&core, "data");

    addBusRegisterConnection(&core, data, A);
    addBusRegisterConnection(&core, data, B);
    addBusRegisterConnection(&core, address, A);
    addBusRegisterConnection(&core, address, B);

    addInstructionRegister(&core, data);
    addMemory64k(&core, address, data);

    addHaltInstruction(&core);

    addCoreLoop(&core, mcode);

    writeCore(&core, filename);
}