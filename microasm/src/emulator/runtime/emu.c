#include "emulator/runtime/emu.h"
#include "shared/memory.h"
#include "shared/platform.h"
#include <stdio.h>

void runEmulator(const char* filename, bool verbose, const char* logFileName) {
    FILE* data = fopen(filename, "rb");

    if(data == NULL) {
        cErrPrintf(TextRed, "Could not open binary file\n");
        return;
    }

    uint16_t* memory = ArenaAlloc(sizeof(uint16_t) * (1<<16));

    unsigned int i = 0;
    while(true) {
        int num1 = fgetc(data);
        int num2 = fgetc(data);
        if(num1 == EOF) {
            break;
        }
        if(num2 == EOF) {
            cErrPrintf(TextRed, "Error reading binary file\n");
        }
        memory[i] = (num1 << 8) + num2;
        i++;
    }

    fclose(data);

    FILE* logFile;
    if(logFileName == NULL) {
        logFile = stdout;
    } else {
        logFile = fopen(logFileName, "w");
    }

    if(verbose) {
        emulatorVerbose(memory, logFile);
    } else {
        emulator(memory);
    }
}
