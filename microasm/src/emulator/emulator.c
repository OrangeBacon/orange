#include "emulator/emulator.h"
#include "shared/memory.h"
#include <stdarg.h>

void pushDepends(Dependancy* dep, size_t count, ...) {
    va_list args;
    va_start(args, count);

    for(size_t i = 0; i < count; i++) {
        PUSH_ARRAY(void*, *dep, dep, va_arg(args, void*)); 
    }
}