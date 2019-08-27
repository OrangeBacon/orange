#include "emulator/compiletime/writeEmulator.h"

#include <stdio.h>

void writeEmulator(const char* filename, Microcode* mcode) {
    FILE* file = fopen(filename, "w");

    fputs("void emulator(){}", file);

    fclose(file);
    (void)mcode;
}