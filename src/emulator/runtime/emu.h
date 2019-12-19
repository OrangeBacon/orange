#ifndef EMU_H
#define EMU_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// allow the emulator to be called from main
void emulator(uint16_t* memory);
void emulatorVerbose(uint16_t* memory, FILE* logFile);
void runEmulator(const char* filename, bool verbose, const char* logFileName);

#endif
