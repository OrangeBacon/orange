#ifndef INSTREG_H
#define INSTREG_H

#include <stdint.h>
#include "emulator/runtime/vmcore.h"

// stores current opcode and arguments to said opcode
typedef struct instructionRegister {
    uint16_t value;
    uint16_t opcode;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t arg3;
    uint16_t arg12;
    uint16_t arg123;
} instructionRegister;

// create an instruction register
unsigned int instRegInit(VMCore* core, instructionRegister* reg, unsigned int instBus);

#endif