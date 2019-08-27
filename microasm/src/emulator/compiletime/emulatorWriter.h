#ifndef EMULATOR_WRITER_H
#define EMULATOR_WRITER_H

#include "emulator/compiletime/cWriter.h"

typedef struct Register {
    const char* name;
} Register;

typedef struct Bus {
    const char* name;
} Bus;

Register* addRegister(cWriter* writer, const char* name);
Bus* addBus(cWriter* writer, const char* name);

#endif