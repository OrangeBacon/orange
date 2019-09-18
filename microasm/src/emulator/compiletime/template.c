#include "emulator/compiletime/template.h"

void createEmulator(VMCoreGen* core) {
    initCore(core);

    unsigned int A = addRegister(core, "A");
    unsigned int B = addRegister(core, "B");
    unsigned int IP = addRegister(core, "IP");
    unsigned int address = addBus(core, "Address");
    unsigned int data = addBus(core, "Data");
    unsigned int instBus = addBus(core, "Inst");

    addBusRegisterConnection(core, data, A, 0);
    addBusRegisterConnection(core, data, B, 0);
    addBusRegisterConnection(core, address, A, 0);
    addBusRegisterConnection(core, address, B, 0);
    addBusRegisterConnection(core, address, IP, 1);

    addInstructionRegister(core, instBus);
    Memory mem = addMemory64k(core, address, data);
    addMemoryBusOutput(core, &mem, instBus);

    addHaltInstruction(core);
}