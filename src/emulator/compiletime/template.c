#include "emulator/compiletime/template.h"

void createEmulator(VMCoreGen* core) {
    initCore(core);

    // TODO: Add ALU

    unsigned int A = addRegister(core, "A");
    unsigned int B = addRegister(core, "B");
    unsigned int C = addRegister(core, "C");
    unsigned int D = addRegister(core, "D");
    unsigned int E = addRegister(core, "E");
    unsigned int AR = addRegister(core, "AR");
    unsigned int IP = addRegister(core, "IP");
    unsigned int SP = addRegister(core, "SP");

    unsigned int address = addBus(core, "Address");
    unsigned int data = addBus(core, "Data");
    unsigned int instBus = addBus(core, "Inst");

    addBusRegisterConnection(core, data, A, 0);
    addBusRegisterConnection(core, data, B, 0);
    addBusRegisterConnection(core, data, C, 0);
    addBusRegisterConnection(core, data, D, 0);
    addBusRegisterConnection(core, data, E, 0);
    addBusRegisterConnection(core, data, AR, 0);
    addBusRegisterConnection(core, data, SP, 0);
    addBusRegisterConnection(core, data, IP, 0);
    addBusRegisterConnection(core, address, A, 0);
    addBusRegisterConnection(core, address, B, 0);
    addBusRegisterConnection(core, address, C, 0);
    addBusRegisterConnection(core, address, D, 0);
    addBusRegisterConnection(core, address, E, 0);
    addBusRegisterConnection(core, address, AR, 0);
    addBusRegisterConnection(core, address, SP, 0);
    addBusRegisterConnection(core, address, IP, 0);

    addBusRegisterConnection(core, address, IP, 1);

    addInstructionRegister(core, instBus);
    Memory mem = addMemory64k(core, address, data);
    addMemoryBusOutput(core, &mem, instBus);

    //addConditionRegister(core);

    addHaltInstruction(core);
}
