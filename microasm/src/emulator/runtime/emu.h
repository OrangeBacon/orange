#ifndef EMU_H
#define EMU_H

#include <stdint.h>
#include <stdbool.h>

// allow the emulator to be called from main
void emulator(uint16_t* memory);
void emulatorVerbose(uint16_t* memory);
void runEmulator(const char* filename, bool verbose);

#endif