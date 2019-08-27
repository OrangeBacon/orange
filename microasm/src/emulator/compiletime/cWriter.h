#ifndef CWRITER_HEADER
#define CWRITER_HEADER

#include "shared/memory.h"
#include "shared/table.h"
#include <stdbool.h>

typedef struct cHeader {
    const char* fileName;
    bool system;
} cHeader;

typedef struct cVariable {
    const char* type;
    const char* name;
} cVariable;

typedef struct cWriter {
    Table headers;
    Table variables;
} cWriter;

void initWriter(cWriter* writer);
void addHeader(cWriter* writer, bool system, const char* header, ...);
void addVariable(cWriter* writer, const char* type, const char* name);
void writeC(const char* fileName, cWriter* writer);

#endif