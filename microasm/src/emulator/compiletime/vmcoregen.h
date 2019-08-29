#ifndef VM_CORE_GEN_H
#define VM_CORE_GEN_H

#include "shared/memory.h"
#include "shared/table.h"

typedef struct Dependancy {
    DEFINE_ARRAY(unsigned int, dep);
} Dependancy;

typedef struct VMCoreGen {
    DEFINE_ARRAY(const char*, compName);

    Table headers;
    DEFINE_ARRAY(const char*, variable);

    DEFINE_ARRAY(const char*, command);
    DEFINE_ARRAY(void*, context);
    DEFINE_ARRAY(Dependancy, depends);
    DEFINE_ARRAY(Dependancy, changes);
} VMCoreGen;

typedef unsigned int Register;
typedef unsigned int Bus;

void initCore(VMCoreGen* core);

void addHeader(VMCoreGen* core, const char* format, ...);
void addVariable(VMCoreGen* core, const char* format, ...);

Register addBus(VMCoreGen* core, const char* name);
Bus addRegister(VMCoreGen* core, const char* name);

void addBusRegisterConnection(VMCoreGen* core, Bus bus, Register reg);

void writeCore(VMCoreGen* core, const char* filename);

#endif