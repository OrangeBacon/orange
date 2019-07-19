#include "emulator/vmcore.h"
#include "emulator/register.h"

void vmcoreInit(VMCore* core) {
    ARRAY_ALLOC(Bus, *core, bus);
    ARRAY_ALLOC(Register, *core, register);
}

BusId createBus(VMCore* core) {
    BusId id = {.id = core->busCount, .core = core};

    Bus bus = {0};
    PUSH_ARRAY(Bus, *core, bus, bus);

    return id;
}

RegisterId createRegister(VMCore* core) {
    RegisterId id = {.id = core->busCount, .core = core};

    Register reg;
    regInit(&reg);
    PUSH_ARRAY(Register, *core, register, reg);
    return id;
}