#include "emulator/compiletime/createEmulator.h"
#include "emulator/compiletime/cWriter.h"
#include "emulator/compiletime/emulatorWriter.h"

void createEmulator(const char* fileName, Microcode* mcode) {
    (void)mcode;

    cWriter writer;
    initWriter(&writer);

    writer.preamble = "void emulator(){\n";
    writer.footer = "}\n";

    addHeader(&writer, "stdio.h", true);
    addHeader(&writer, "stdbool.h", true);

    addRegister(&writer, "A");
    addRegister(&writer, "B");

    writeC(fileName, &writer);
}