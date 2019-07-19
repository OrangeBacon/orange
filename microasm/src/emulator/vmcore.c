#include "emulator/vmcore.h"
#include "emulator/register.h"

void vmcoreInit(VMCore* core) {
    ARRAY_ALLOC(Bus, *core, bus);
}

BusId createBus(VMCore* core) {
    Bus bus = {0};
    PUSH_ARRAY(Bus, *core, bus, bus);
    BusId id = {0};
    return id;
}

RegisterId createRegister(VMCore* core) {
    (void)core;
    RegisterId id = {0};
    return id;
}