#include "shared/memory.h"
#include <stdbool.h>

typedef struct cHeader {
    const char* fileName;
    bool system;
} cHeader;

typedef struct cWriter {
    const char* preamble;
    DEFINE_ARRAY(cHeader, header);
} cWriter;

void initWriter(cWriter* writer);
void addHeader(cWriter* writer, const char* header, bool system);
void writeC(const char* fileName, cWriter* writer);