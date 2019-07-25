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

typedef struct Dependancy {
    DEFINE_ARRAY(void*, dep);
} Dependancy;

typedef struct VMCore {
    DEFINE_ARRAY(Bus, bus);
    DEFINE_ARRAY(Register, register);
    DEFINE_ARRAY(Command, command);
    DEFINE_ARRAY(void*, context);
    DEFINE_ARRAY(Dependancy, depends);
    DEFINE_ARRAY(Dependancy, changes);
} VMCore;

void pushDepends(Dependancy* dep, size_t count, ...);

#define ADD_COMMAND(cmd, ctx, ...) \
    do { \
    PUSH_ARRAY(Command, *core, command, cmd); \
    PUSH_ARRAY(void*, *core, context, ctx); \
    DEPENDS(__VA_ARGS__); \
    } while(0)

#define DEPENDS(...) \
    do { \
    Dependancy depends; \
    ARRAY_ALLOC(void*, depends, dep); \
    pushDepends(&depends, sizeof((void*[]){__VA_ARGS__})/sizeof(void*), __VA_ARGS__);\
    PUSH_ARRAY(Dependancy, *core, depends, depends); \
    } while(0)

#define CHANGES(...) \
    do { \
    Dependancy changes; \
    ARRAY_ALLOC(void*, changes, dep); \
    pushDepends(&changes, sizeof((void*[]){__VA_ARGS__})/sizeof(void*), __VA_ARGS__);\
    PUSH_ARRAY(Dependancy, *core, changes, changes); \
    } while(0)

#endif