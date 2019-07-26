#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>
#include "shared/memory.h"

// The main data structures required by the emulator
// Combined here because I could not figure out how to
// put them in relavant files without recursive includes

// a bus linking several elements together
typedef struct Bus {
    // last output value 
    uint16_t value;
    // has a value been output on the last clock cycle
    bool isValid;
} Bus;

// temporary data storage
typedef struct Register {
    uint16_t value;
} Register;

// defined below
struct VMCore;

// a microcode command to be executed
typedef void(*Command)(struct VMCore* core, void*);

// list of objects that a microcode command depends on
typedef struct Dependancy {
    DEFINE_ARRAY(void*, dep);
} Dependancy;

// the main state for a vm
typedef struct VMCore {
    // all busses in the vm
    DEFINE_ARRAY(Bus, bus);

    // all registers in the vm
    DEFINE_ARRAY(Register, register);

    // a microcode command, the context required as no closures in c
    // and what the command depends upon to execute and changes
    // while running
    DEFINE_ARRAY(Command, command);
    DEFINE_ARRAY(void*, context);
    DEFINE_ARRAY(Dependancy, depends);
    DEFINE_ARRAY(Dependancy, changes);

    // which phase of execution in an assembly instruction
    // is the vm at, relies on overflowing to 0 instead of
    // having a value of 16 (is this undefined behaviour?)
    unsigned int phase : 4;
} VMCore;

// va args array push wrapper
void pushDepends(Dependancy* dep, size_t count, ...);

// add a microcode command to the core
// with all of the objects it depends on.
// CHANGES must be called imediatly afterwards
// the core must be a pointer to VMCore called "core"
// Depended and changed objects are pointers to objects
#define ADD_COMMAND(cmd, ctx, ...) \
    do { \
    PUSH_ARRAY(Command, *core, command, cmd); \
    PUSH_ARRAY(void*, *core, context, ctx); \
    DEPENDS(__VA_ARGS__); \
    } while(0)

// Adds the dependancies provided to the core
// TODO: fix if no dependancies provided
#define DEPENDS(...) \
    do { \
    Dependancy depends; \
    ARRAY_ALLOC(void*, depends, dep); \
    pushDepends(&depends, sizeof((void*[]){__VA_ARGS__})/sizeof(void*), __VA_ARGS__);\
    PUSH_ARRAY(Dependancy, *core, depends, depends); \
    } while(0)

// Adds the changed opjects provided to the core
// must be called after add_command so array counts
// match commands to changes correctly
// TODO: fix if no dependancies provided
#define CHANGES(...) \
    do { \
    Dependancy changes; \
    ARRAY_ALLOC(void*, changes, dep); \
    pushDepends(&changes, sizeof((void*[]){__VA_ARGS__})/sizeof(void*), __VA_ARGS__);\
    PUSH_ARRAY(Dependancy, *core, changes, changes); \
    } while(0)

#endif