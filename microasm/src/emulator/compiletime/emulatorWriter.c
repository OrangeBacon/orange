#include "emulator/compiletime/emulatorWriter.h"
#include "shared/memory.h"

Register* addRegister(cWriter* writer, const char* name) {
    addHeader(writer, false, "emulator/runtime/register.h");
    addVariable(writer, "uint32_t", name);
    addInitCode(writer, "(void)%s;", name);

    Register* reg = ArenaAlloc(sizeof(Register));
    return reg;
}

Bus* addBus(cWriter* writer, const char* name) {
    addHeader(writer, false, "emulator/runtime/emulator.h");
    addVariable(writer, "uint32_t", name);
    addInitCode(writer, "(void)%s;", name);

    Bus* bus = ArenaAlloc(sizeof(Bus));
    return bus;
}