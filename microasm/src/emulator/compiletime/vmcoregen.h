#ifndef VM_CORE_GEN_H
#define VM_CORE_GEN_H

#include "shared/memory.h"
#include "shared/table.h"

typedef struct VMCoreGen {
    Table headers;
    DEFINE_ARRAY(const char*, variable);
} VMCoreGen;

void initCore(VMCoreGen* core);

void addHeader(VMCoreGen* core, const char* format, ...);
void addVariable(VMCoreGen* core, const char* format, ...);

void addRegister(VMCoreGen* core, const char* name);

void writeCore(VMCoreGen* core, const char* filename);

#endif