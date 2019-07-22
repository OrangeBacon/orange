#include "emulator/vmcore.h"
#include "emulator/register.h"

void vmcoreInit(VMCore* core) {
    ARRAY_ALLOC(Bus, *core, bus);
    ARRAY_ALLOC(Register, *core, register);
}

unsigned int createBus(VMCore* core) {
    Bus bus = {0};
    PUSH_ARRAY(Bus, *core, bus, bus);

    return core->busCount - 1;
}

unsigned int createRegister(VMCore* core) {
    Register reg;
    regInit(&reg);
    PUSH_ARRAY(Register, *core, register, reg);
    return core->registerCount - 1;
}