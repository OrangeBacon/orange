#include "emulator/runtime/emu.h"
#include "shared/memory.h"
#include "shared/platform.h"
#include <stdio.h>

void runEmulator(const char* filename) {
    FILE* file = fopen(filename, "rb");

    uint16_t* memory = ArenaAlloc(sizeof(uint16_t) * (1<<16));

    unsigned int i = 0;
    while(true) {
        int num1 = fgetc(file);
        int num2 = fgetc(file);
        if(num1 == EOF) {
            break;
        }
        if(num2 == EOF) {
            cErrPrintf(TextRed, "Error reading binary file");
        }
        memory[i] = (num1 << 8) + num2;
        i++;
    }

    emulator(memory);

    fclose(file);
}