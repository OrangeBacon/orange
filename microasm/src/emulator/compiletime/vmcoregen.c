#include "emulator/compiletime/vmcoregen.h"

#include <stdio.h>
#include <stdarg.h>

void initCore(VMCoreGen* core) {
    ARRAY_ALLOC(const char*, *core, compName);
    ARRAY_ALLOC(const char*, *core, variable);
    ARRAY_ALLOC(const char*, *core, command);
    ARRAY_ALLOC(Arguments, *core, argument);
    ARRAY_ALLOC(Dependancy, *core, depends);
    ARRAY_ALLOC(Dependancy, *core, changes);
    initTable(&core->headers, strHash, strCmp);
}

void addHeader(VMCoreGen* core, const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t len = vsnprintf(NULL, 0, format, args);
    char* buf = ArenaAlloc(len * sizeof(char) + 1);
    vsnprintf(buf, len + 1, format, args);

    tableSet(&core->headers, buf, (void*)1);
}

void addVariable(VMCoreGen* core, const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t len = vsnprintf(NULL, 0, format, args);
    char* buf = ArenaAlloc(len * sizeof(char) + 1);
    vsnprintf(buf, len + 1, format, args);

    PUSH_ARRAY(const char*, *core, variable, buf);
}

Register addRegister(VMCoreGen* core, const char* name) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t %s", name);
    PUSH_ARRAY(const char*, *core, compName, name);
    return core->compNameCount - 1;
}

Bus addBus(VMCoreGen* core, const char* name) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t %s", name);
    PUSH_ARRAY(const char*, *core, compName, name);
    return core->compNameCount - 1;
}

void addInstructionRegister(VMCoreGen* core, Bus iBus) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t value");
    addVariable(core, "uint16_t opcode");
    addVariable(core, "uint16_t arg1");
    addVariable(core, "uint16_t arg2");
    addVariable(core, "uint16_t arg3");
    addVariable(core, "uint16_t arg12");
    addVariable(core, "uint16_t arg123");
    PUSH_ARRAY(const char*, *core, compName, "IReg");
    unsigned int this = core->compNameCount - 1;

    {
        PUSH_ARRAY(const char*, *core, command, "emulator/runtime/instRegSet.c");
        Arguments args;
        ARRAY_ALLOC(Argument, args, arg);
        PUSH_ARRAY(Argument, args, arg, ((Argument){.name = "inst", .value = core->compNames[iBus]}));
        PUSH_ARRAY(Arguments, *core, argument, args);
        Dependancy depends;
        ARRAY_ALLOC(unsigned int, depends, dep);
        PUSH_ARRAY(unsigned int, depends, dep, iBus);
        PUSH_ARRAY(Dependancy, *core, depends, depends);
        Dependancy changes;
        ARRAY_ALLOC(unsigned int, changes, dep);
        PUSH_ARRAY(unsigned int, changes, dep, this);
        PUSH_ARRAY(Dependancy, *core, changes, changes);
    }
}

void addBusRegisterConnection(VMCoreGen* core, Bus bus, Register reg) {
    {
        PUSH_ARRAY(const char*, *core, command, "emulator/runtime/busToReg.c");
        Arguments args;
        ARRAY_ALLOC(Argument, args, arg);
        PUSH_ARRAY(Argument, args, arg, ((Argument){.name = "BUS", .value = core->compNames[bus]}));
        PUSH_ARRAY(Argument, args, arg, ((Argument){.name = "REGISTER", .value = core->compNames[reg]}));
        PUSH_ARRAY(Arguments, *core, argument, args);
        Dependancy depends;
        ARRAY_ALLOC(unsigned int, depends, dep);
        PUSH_ARRAY(unsigned int, depends, dep, bus);
        PUSH_ARRAY(Dependancy, *core, depends, depends);
        Dependancy changes;
        ARRAY_ALLOC(unsigned int, changes, dep);
        PUSH_ARRAY(unsigned int, changes, dep, reg);
        PUSH_ARRAY(Dependancy, *core, changes, changes);
    }

    {
        PUSH_ARRAY(const char*, *core, command, "emulator/runtime/regToBus.c");
        Arguments args;
        ARRAY_ALLOC(Argument, args, arg);
        PUSH_ARRAY(Argument, args, arg, ((Argument){.name = "BUS", .value = core->compNames[bus]}));
        PUSH_ARRAY(Argument, args, arg, ((Argument){.name = "REGISTER", .value = core->compNames[reg]}));
        PUSH_ARRAY(Arguments, *core, argument, args);
        Dependancy depends;
        ARRAY_ALLOC(unsigned int, depends, dep);
        PUSH_ARRAY(unsigned int, depends, dep, reg);
        PUSH_ARRAY(Dependancy, *core, depends, depends);
        Dependancy changes;
        ARRAY_ALLOC(unsigned int, changes, dep);
        PUSH_ARRAY(unsigned int, changes, dep, bus);
        PUSH_ARRAY(Dependancy, *core, changes, changes);
    }
}

void writeCore(VMCoreGen* core, const char* filename) {
    FILE* file = fopen(filename, "w");

    for(unsigned int i = 0; i < core->headers.capacity; i++) {
        Entry* entry = &core->headers.entries[i];
        const char* header = entry->key.value;
        if(header == NULL) {
            continue;
        }
        fprintf(file, "#include %s\n", header);
    }

    fputs("void emulator() {\n", file);

    for(unsigned int i = 0; i < core->variableCount; i++) {
        fprintf(file, "%s = {0};\n", core->variables[i]);
    }

    for(unsigned int i = 0; i < core->commandCount; i++) {
        for(unsigned int j = 0; j < core->arguments[i].argCount; j++) {
            Argument* arg = &core->arguments[i].args[j];
            fprintf(file, "#define %s %s\n", arg->name, arg->value);
        }
        fprintf(file, "#include \"%s\"\n", core->commands[i]);
        for(unsigned int j = 0; j < core->arguments[i].argCount; j++) {
            Argument* arg = &core->arguments[i].args[j];
            fprintf(file, "#undef %s\n", arg->name);
        }
    }

    fputs("}\n", file);

    fclose(file);
}