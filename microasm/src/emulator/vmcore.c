#include "emulator/vmcore.h"
#include "emulator/register.h"

void vmcoreInit(VMCore* core) {
    ARRAY_ALLOC(Bus, *core, bus);
    ARRAY_ALLOC(Register, *core, register);
    ARRAY_ALLOC(Command, *core, command);
    ARRAY_ALLOC(void*, *core, context);
    ARRAY_ALLOC(Dependancy, *core, depends);
    ARRAY_ALLOC(Dependancy, *core, changes);
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

void coreCall(VMCore* core, unsigned int method) {
    core->commands[method](core, core->contexts[method]);
}

void coreCallLine(VMCore* core, unsigned int count, ...) {
    va_list args;
    va_start(args, count);

    for(unsigned int i = 0; i < count; i++) {
        coreCall(core, va_arg(args, int));
    }

    va_end(args);
}