#include <string.h>

#include "shared/platform.h"
#include "shared/memory.h"
#include "microcode/codegen.h"

void codegen(Microcode* m, char* outputFile) {
    char* filename;
    const char* dot = strrchr(outputFile, '.');
    if(dot == NULL) {
        size_t len = strlen(outputFile) + 7;
        filename = ArenaAlloc(sizeof(char) * len);
        strncpy(filename, outputFile, len - 7);
        strcat(filename, ".uasmb");
        filename[len - 1] = '\0';
    } else {
        filename = outputFile;
    }
    cOutPrintf(TextWhite, "CodeGen: Opcode[%u]->%s", m->opcodeCount, filename);
}