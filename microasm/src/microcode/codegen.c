#include <math.h>
#include <string.h>
#include <stdio.h>

#include "shared/platform.h"
#include "shared/memory.h"
#include "microcode/codegen.h"

void codegen(Microcode* m, char* outputFile) {
    char* filename;
    const char* dot = strrchr(outputFile, '.');
    const char* slash = strrchr(outputFile, '/');
    bool bsErr = false;
    if(pathSeperator[0] == '\\') {
        const char* backslash = strrchr(outputFile, '\\');
        bsErr = backslash > dot;
    }
    if(dot == NULL || slash > dot || bsErr) {
        size_t len = strlen(outputFile) + 7;
        filename = ArenaAlloc(sizeof(char) * len);
        strncpy(filename, outputFile, len - 7);
        strcat(filename, ".uasmb");
        filename[len - 1] = '\0';
    } else {
        filename = outputFile;
    }
    cOutPrintf(TextWhite, "CodeGen: Opcode[%u]->%s\n", m->opcodeCount, filename);
    for(unsigned int i = 0; i < m->opcodeCount; i++) {
        cOutPrintf(TextWhite, "  code %u\n", m->opcodes[i].id);
    }

    FILE* file = fopen(filename, "wb+");
    // magic numbers
    unsigned int num = 0xCDAB;
    fwrite(&num, 2, 1, file);

    // version
    num = 0x0000;
    fwrite(&num, 2, 1, file);

    // number of input bits
    fwrite(&m->inputBitCount, 2, 1, file);

    // number of outputs bits
    fwrite(&m->outputBitCount, 2, 1, file);
    
    unsigned int outputByteCount = m->outputBitCount / 8 + (m->outputBitCount % 8 != 0);

    for(unsigned int i = 0; i < pow(2, m->opcodeSize + m->phaseSize); i++) {
        unsigned int opcode = i >> m->phaseSize;
        unsigned int phase = i & (unsigned int)pow(2, m->phaseSize);
        for(unsigned int j = 0; j < outputByteCount; j++) {
            fputc(opcode + phase, file);
        }
    }
    fclose(file);
}