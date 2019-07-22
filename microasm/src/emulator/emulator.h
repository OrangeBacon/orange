#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>
#include "shared/memory.h"

typedef struct Bus {
    uint16_t value;
} Bus;

typedef struct Register {
    uint16_t value;
} Register;

struct VMCore;

typedef void(*Command)(struct VMCore* core, void*);

typedef struct VMCore {
    DEFINE_ARRAY(Bus, bus);
    DEFINE_ARRAY(Register, register);
    DEFINE_ARRAY(Command, command);
    DEFINE_ARRAY(void*, context);
} VMCore;

#endif