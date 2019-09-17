#include "emulator/compiletime/create.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void initCore(VMCoreGen* core) {
    ARRAY_ALLOC(const char*, *core, compName);
    ARRAY_ALLOC(const char*, *core, variable);
    ARRAY_ALLOC(const char*, *core, command);
    ARRAY_ALLOC(Command, *core, command);
    initTable(&core->headers, strHash, strCmp);
    core->codeIncludeBase = "emulator/runtime/";
    addHeader(core, "<stdbool.h>");
}

unsigned int* AllocUInt(unsigned int itemCount, ...) {
    va_list args;
    va_start(args, itemCount);

    unsigned int* data = ArenaAlloc(sizeof(unsigned int) * itemCount);

    for(unsigned int i = 0; i < itemCount; i++) {
        data[i] = va_arg(args, unsigned int);
    }

    return data;
}

Argument* AllocArgument(unsigned int itemCount, ...) {
    va_list args;
    va_start(args, itemCount);

    Argument* data = ArenaAlloc(sizeof(Argument) * itemCount);

    for(unsigned int i = 0; i < itemCount; i++) {
        data[i] = va_arg(args, Argument);
    }

    return data;
}

void addCommand(VMCoreGen* core, Command command) {
    PUSH_ARRAY(Command, *core, command, command);
}

char* aprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t len = vsnprintf(NULL, 0, format, args);
    char* buf = ArenaAlloc(len * sizeof(char) + 1);
    vsnprintf(buf, len + 1, format, args);

    return buf;
}

void addHeader(VMCoreGen* core, const char* header) {
    tableSet(&core->headers, (void*)header, (void*)1);
}

void addVariable(VMCoreGen* core, const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t len = vsnprintf(NULL, 0, format, args);
    char* buf = ArenaAlloc(len * sizeof(char) + 1);
    vsnprintf(buf, len + 1, format, args);

    PUSH_ARRAY(const char*, *core, variable, buf);
}

unsigned int addRegister(VMCoreGen* core, const char* name) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t %s", name);
    PUSH_ARRAY(const char*, *core, compName, name);
    return core->compNameCount - 1;
}

unsigned int addBus(VMCoreGen* core, const char* name) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t %s", name);
    PUSH_ARRAY(const char*, *core, compName, name);
    return core->compNameCount - 1;
}

void addInstructionRegister(VMCoreGen* core, unsigned int iBus) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t opcode");
    addVariable(core, "uint16_t arg1");
    addVariable(core, "uint16_t arg2");
    addVariable(core, "uint16_t arg3");
    addVariable(core, "uint16_t arg12");
    addVariable(core, "uint16_t arg123");
    PUSH_ARRAY(const char*, *core, compName, "IReg");
    unsigned int this = core->compNameCount - 1;

    addCommand(core, (Command) {
        .name = "iRegSet",
        .file = "instRegSet",
        ARGUMENTS(((Argument){.name = "inst", .value = core->compNames[iBus]})),
        DEPENDS(iBus),
        CHANGES(this)
    });
}

Memory addMemory64k(VMCoreGen* core, unsigned int address, unsigned int data) {
    addHeader(core, "<stdint.h>");
    PUSH_ARRAY(const char*, *core, compName, "Memory64");
    unsigned int this = core->compNameCount - 1;

    addCommand(core, (Command) {
        .name = aprintf("memReadTo%s", core->compNames[data]),
        .file = "memRead",
        ARGUMENTS(
            ((Argument){.name = "data", .value = core->compNames[data]}), 
            ((Argument){.name = "address", .value = core->compNames[address]})),
        DEPENDS(address, this),
        CHANGES(data)
    });

    addCommand(core, (Command) {
        .name = "memWrite",
        .file = "memWrite",
        ARGUMENTS(
            ((Argument){.name = "data", .value = core->compNames[data]}), 
            ((Argument){.name = "address", .value = core->compNames[address]})),
        DEPENDS(address, data),
        CHANGES(this)
    });

    return (Memory){
        .id = this,
        .address = address
    };
}

void addMemoryBusOutput(VMCoreGen* core, Memory* mem, unsigned int bus) {
    addCommand(core, (Command) {
        .name = aprintf("memReadTo%s", core->compNames[bus]),
        .file = "memRead",
        ARGUMENTS(
            ((Argument){.name = "data", .value = core->compNames[bus]}), 
            ((Argument){.name = "address", .value = core->compNames[mem->address]})),
        DEPENDS(mem->address, mem->id),
        CHANGES(bus)
    });
}

void addBusRegisterConnection(VMCoreGen* core, unsigned int bus, unsigned int reg, int state) {
    if(state == -1 || state == 0) {
        addCommand(core, (Command) {
            .name = aprintf("%sTo%s", core->compNames[bus], core->compNames[reg]),
            .file = "busToReg",
            ARGUMENTS(
                ((Argument){.name = "BUS", .value = core->compNames[bus]}), 
                ((Argument){.name = "REGISTER", .value = core->compNames[reg]})),
            DEPENDS(bus),
            CHANGES(reg)
        });
    }

    if(state == 0 || state == 1) {
        addCommand(core, (Command) {
            .name = aprintf("%sTo%s", core->compNames[reg], core->compNames[bus]),
            .file = "regToBus",
            ARGUMENTS(
                ((Argument){.name = "BUS", .value = core->compNames[bus]}), 
                ((Argument){.name = "REGISTER", .value = core->compNames[reg]})),
            DEPENDS(reg),
            CHANGES(bus)
        });
    }
}

void addHaltInstruction(VMCoreGen* core) {
    addHeader(core, "<stdlib.h>");
    addHeader(core, "<stdio.h>");
    addCommand(core, (Command) {
        .name = "halt",
        .file = "halt"
    });
}