#ifndef EMU_H
#define EMU_H

#include <stdint.h>

// allow the emulator to be called from main
void emulator(uint16_t* memory);
void runEmulator(const char* filename);

#endif