#ifndef VM_CORE_GEN_H
#define VM_CORE_GEN_H

#include "shared/memory.h"
#include "shared/table.h"

typedef struct Argument {
    const char* name;
    const char* value;
} Argument;

typedef struct GenOpCodeLine {
    bool hasCondition;
    DEFINE_ARRAY(unsigned int, highBit);
    DEFINE_ARRAY(unsigned int, lowBit);
} GenOpCodeLine;

typedef struct GenOpCode {
    unsigned int id;
    const char* name;
    unsigned int nameLen;
    DEFINE_ARRAY(GenOpCodeLine*, line);
    bool isValid;
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

    unsigned int* reads;
    unsigned int readsLength;

    unsigned int* writes;
    unsigned int writesLength;
} Command;

#define FOREACH_COMPONENT(x) \
    x(BUS) x(REGISTER) x(OTHER)

#define ENUM_COMPONENT(x) COMPONENT_##x,
#define ADD_COMPONENT(x) +1

typedef enum ComponentType {
    FOREACH_COMPONENT(ENUM_COMPONENT)
} ComponentType;

const char* ComponentTypeNames[FOREACH_COMPONENT(ADD_COMPONENT)];

#undef ENUM_COMPONENT
#undef ADD_COMPONENT

typedef struct Component {
    const char* name;
    ComponentType type;

    bool busStatus;
} Component;

typedef struct VMCoreGen {
    DEFINE_ARRAY(Component, component);

    Table headers;
    DEFINE_ARRAY(const char*, variable);
    DEFINE_ARRAY(const char*, loopVariable);

    DEFINE_ARRAY(Command, command);

    GenOpCode* opcodes;
    unsigned int opcodeCount;

    DEFINE_ARRAY(unsigned int, headBit);

    const char* codeIncludeBase;
} VMCoreGen;

typedef struct Memory {
    unsigned int address;
    unsigned int id;
} Memory;

void initCore(VMCoreGen* core);

void addHeader(VMCoreGen* core, const char* header);
void addVariable(VMCoreGen* core, const char* format, ...);
void addLoopVariable(VMCoreGen* core, const char* format, ...);

unsigned int addBus(VMCoreGen* core, const char* name);
unsigned int addRegister(VMCoreGen* core, const char* name);
void addInstructionRegister(VMCoreGen* core, unsigned int iBus);
void addConditionRegister(VMCoreGen* core);
void addHaltInstruction(VMCoreGen* core);

void addBusRegisterConnection(VMCoreGen* core, unsigned int bus, unsigned int reg, int state);
Memory addMemory64k(VMCoreGen* core, unsigned int address, unsigned int data);
void addMemoryBusOutput(VMCoreGen* core, Memory* mem, unsigned int bus);

void addCommand(VMCoreGen* core, Command command);

unsigned int* AllocUInt(unsigned int itemCount, ...);
Argument* AllocArgument(unsigned int itemCount, ...);

#define STRUCT_ARRAY(type, name, alloc, ...) \
    .name = alloc(sizeof((type[]){__VA_ARGS__})/sizeof(type), __VA_ARGS__), \
    .name##Length = sizeof((type[]){__VA_ARGS__})/sizeof(type)

#define DEPENDS(...) \
    STRUCT_ARRAY(unsigned int, depends, AllocUInt, __VA_ARGS__)

#define CHANGES(...) \
    STRUCT_ARRAY(unsigned int, changes, AllocUInt, __VA_ARGS__)

#define ARGUMENTS(...) \
    STRUCT_ARRAY(Argument, args, AllocArgument, __VA_ARGS__)

#define BUS_READ(...) \
    STRUCT_ARRAY(unsigned int, reads, AllocUInt, __VA_ARGS__)

#define BUS_WRITE(...) \
    STRUCT_ARRAY(unsigned int, writes, AllocUInt, __VA_ARGS__)

#endif
