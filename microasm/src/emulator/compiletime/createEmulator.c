#include "emulator/compiletime/createEmulator.h"
#include "emulator/compiletime/cWriter.h"

void createEmulator(const char* fileName, Microcode* mcode) {
    (void)mcode;

    cWriter writer;
    initWriter(&writer);

    addHeader(&writer, "stdio.h", true);
    addHeader(&writer, "stdbool.h", true);

    writer.preamble =
        "void emulator() {\n"
        "switch(true){\n"
        "case false: break;\n"
        "case true: break;\n"
        "}\n"
        "}\n";

    writeC(fileName, &writer);
}