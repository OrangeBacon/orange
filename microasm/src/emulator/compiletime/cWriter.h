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
    const char* preamble;
    Table headers;
    Table variables;
    DEFINE_ARRAY(const char*, initCode);
    const char* footer;
} cWriter;

void initWriter(cWriter* writer);
void addHeader(cWriter* writer, const char* header, bool system);
void addVariable(cWriter* writer, const char* type, const char* name);
void addInitCode(cWriter* writer, const char* code, ...);
void writeC(const char* fileName, cWriter* writer);

#endif