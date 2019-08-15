#include "emulator/compiletime/emulatorWriter.h"
#include "shared/memory.h"

Register* addRegister(cWriter* writer, const char* name) {
    addHeader(writer, "emulator/runtime/register.h", false);
    addVariable(writer, "Register", name);
    addInitCode(writer, "(void)%s;", name);

    Register* reg = ArenaAlloc(sizeof(Register));
    return reg;
}