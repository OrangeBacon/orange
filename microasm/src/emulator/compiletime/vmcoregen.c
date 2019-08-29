#include "emulator/compiletime/vmcoregen.h"

#include <stdio.h>
#include <stdarg.h>

void initCore(VMCoreGen* core) {
    ARRAY_ALLOC(const char*, *core, compName);
    ARRAY_ALLOC(const char*, *core, variable);
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
    addVariable(core, "uint16_t register%s", name);
    PUSH_ARRAY(const char*, *core, compName, name);
    return core->compNameCount - 1;
}

Bus addBus(VMCoreGen* core, const char* name) {
    addHeader(core, "<stdint.h>");
    addVariable(core, "uint16_t bus%s", name);
    PUSH_ARRAY(const char*, *core, compName, name);
    return core->compNameCount - 1;
}

void addBusRegisterConnection(VMCoreGen* core, Bus bus, Register reg) {
    (void)core;
    (void)bus;
    (void)reg;
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
        fprintf(file, "%s;\n", core->variables[i]);
    }

    fputs("}\n", file);
    fclose(file);
}