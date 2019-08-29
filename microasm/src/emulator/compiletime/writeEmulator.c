#include "emulator/compiletime/writeEmulator.h"

#include "emulator/compiletime/vmcoregen.h"

void writeEmulator(const char* filename, Microcode* mcode) {
    (void)mcode;

    VMCoreGen core;
    initCore(&core);

    addRegister(&core, "A");
    addRegister(&core, "B");

    writeCore(&core, filename);
}