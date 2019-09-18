#ifndef VM_CORE_GEN_H
#define VM_CORE_GEN_H

#include "shared/memory.h"
#include "shared/table.h"

typedef struct Argument {
    const char* name;
    const char* value;
} Argument;

typedef struct GenOpCode {
    bool isValid;
    const char* name;
    unsigned int nameLen;
    unsigned int bitCount;
    unsigned int* bits;
} GenOpCode;

typedef struct Command {
    const char* name;
    const char* file;
    Argument* args;
    unsigned int argsLength;
    unsigned int* depends;
    unsigned int dependsLength;
    unsigned int* changes;
    unsigned int changesLength;
} Command;

typedef struct Component {
    const char* name;
    bool busStatus;
} Component;

typedef struct VMCoreGen {
    DEFINE_ARRAY(Component, component);

    Table headers;
    DEFINE_ARRAY(const char*, variable);

    DEFINE_ARRAY(Command, command);

    unsigned int opcodeCount;
    GenOpCode* opcodes;

    unsigned int headCount;
    unsigned int* headBits;

    const char* codeIncludeBase;
} VMCoreGen;

typedef struct Memory {
    unsigned int address;
    unsigned int id;
} Memory;

void initCore(VMCoreGen* core);

void addHeader(VMCoreGen* core, const char* header);
void addVariable(VMCoreGen* core, const char* format, ...);

unsigned int addBus(VMCoreGen* core, const char* name);
unsigned int addRegister(VMCoreGen* core, const char* name);
void addInstructionRegister(VMCoreGen* core, unsigned int iBus);
void addHaltInstruction(VMCoreGen* core);

void addBusRegisterConnection(VMCoreGen* core, unsigned int bus, unsigned int reg, int state);
Memory addMemory64k(VMCoreGen* core, unsigned int address, unsigned int data);
void addMemoryBusOutput(VMCoreGen* core, Memory* mem, unsigned int bus);

void addCommand(VMCoreGen* core, Command command);

unsigned int* AllocUInt(unsigned int itemCount, ...);
Argument* AllocArgument(unsigned int itemCount, ...);

#define DEPENDS(...) \
    .depends = AllocUInt(sizeof((unsigned int[]){__VA_ARGS__})/sizeof(unsigned int), __VA_ARGS__), \
    .dependsLength = sizeof((unsigned int[]){__VA_ARGS__})/sizeof(unsigned int)

#define CHANGES(...) \
    .changes = AllocUInt(sizeof((unsigned int[]){__VA_ARGS__})/sizeof(unsigned int), __VA_ARGS__), \
    .changesLength = sizeof((unsigned int[]){__VA_ARGS__})/sizeof(unsigned int)

#define ARGUMENTS(...) \
    .args = AllocArgument(sizeof((Argument[]){__VA_ARGS__})/sizeof(Argument), __VA_ARGS__), \
    .argsLength = sizeof((Argument[]){__VA_ARGS__})/sizeof(Argument)

#endif