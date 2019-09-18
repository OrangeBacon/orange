#include "emulator/compiletime/codegen.h"

#include <stdio.h>

static void outputLoop(VMCoreGen* core, FILE* file) {
    for(unsigned int i = 0; i < core->variableCount; i++) {
        fprintf(file, "%s = {0};\n", core->variables[i]);
    }

    fputs("while(true) {\n", file);

    for(unsigned int i = 0; i < core->headCount; i++) {
        unsigned int command = core->headBits[i];
        for(unsigned int k = 0; k < core->commands[command].argsLength; k++) {
            Argument* arg = &core->commands[command].args[k];
            fprintf(file, "#define %s %s\n", arg->name, arg->value);
        }
        fprintf(file, "#include \"%s%s.c\"\n", core->codeIncludeBase, core->commands[command].file);
        for(unsigned int k = 0; k < core->commands[command].argsLength; k++) {
            Argument* arg = &core->commands[command].args[k];
            fprintf(file, "#undef %s\n", arg->name);
        }
    }

    fputs("switch(opcode) {\n", file);

    for(unsigned int i = 0; i < core->opcodeCount; i++) {
        GenOpCode* code = &core->opcodes[i];
        if(code->isValid) {
            fprintf(file, "// %.*s\ncase %u: // %u commands\n", code->nameLen, code->name, i, code->bitCount);
            for(unsigned int j = 0; j < code->bitCount; j++) {
                unsigned int command = code->bits[j];
                for(unsigned int k = 0; k < core->commands[command].argsLength; k++) {
                    Argument* arg = &core->commands[command].args[k];
                    fprintf(file, "#define %s %s\n", arg->name, arg->value);
                }
                fprintf(file, "#include \"%s%s.c\"\n", core->codeIncludeBase, core->commands[command].file);
                for(unsigned int k = 0; k < core->commands[command].argsLength; k++) {
                    Argument* arg = &core->commands[command].args[k];
                    fprintf(file, "#undef %s\n", arg->name);
                }
            }
            fputs("break;\n", file);
        }
    }

    fputs("default: exit(0);\n", file);

    fputs("}\nIP++;\n}}\n", file);
}

void coreCodegen(VMCoreGen* core, const char* filename) {
    FILE* file = fopen(filename, "w");

    for(unsigned int i = 0; i < core->headers.capacity; i++) {
        Entry* entry = &core->headers.entries[i];
        const char* header = entry->key.value;
        if(header == NULL) {
            continue;
        }
        fprintf(file, "#include %s\n", header);
    }

    fputs("void emulator(uint16_t* memory) {\n", file);
    outputLoop(core, file);

    fputs("#define DEBUG_OUTPUT\n", file);
    fputs("void emulatorVerbose(uint16_t* memory, FILE* logFile) {\n", file);
    outputLoop(core, file);

    fclose(file);
}