#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>
#include "shared/memory.h"

typedef struct BusId {
    unsigned int id;
} BusId;

typedef struct Bus {
    uint16_t value;
} Bus;

typedef struct RegisterId {
    unsigned int id;
} RegisterId;

typedef struct Register {
    uint16_t value;
    DEFINE_ARRAY(Bus*, bus);
} Register;

typedef struct VMCore {
    DEFINE_ARRAY(Bus, bus);
    DEFINE_ARRAY(Register, register);
} VMCore;

#endif