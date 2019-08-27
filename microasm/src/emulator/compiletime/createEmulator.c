#include "emulator/compiletime/createEmulator.h"
#include "emulator/compiletime/cWriter.h"
#include "emulator/compiletime/emulatorWriter.h"

void createEmulator(const char* fileName, Microcode* mcode) {
    (void)mcode;

    cWriter writer;
    initWriter(&writer);

    writer.preamble = "void emulator(){\n";
    writer.footer = "}\n";

    addHeader(&writer, true, "stdio.h");
    addHeader(&writer, true, "stdbool.h");

    addRegister(&writer, "A");
    addRegister(&writer, "B");
    addBus(&writer, "data");

    writeC(fileName, &writer);
}