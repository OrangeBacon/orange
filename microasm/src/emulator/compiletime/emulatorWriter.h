#ifndef EMULATOR_WRITER_H
#define EMULATOR_WRITER_H

#include "emulator/compiletime/cWriter.h"

typedef struct Register {
    const char* name;
} Register;

Register* addRegister(cWriter* writer, const char* name);

#endif