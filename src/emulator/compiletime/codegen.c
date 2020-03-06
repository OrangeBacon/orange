#include "emulator/compiletime/codegen.h"
#include "shared/log.h"

#include <stdio.h>

static void outputCommand(VMCoreGen* core, FILE* file, unsigned int command) {
    CONTEXT(INFO, "Command = %u", command);
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

static void outputLoop(VMCoreGen* core, FILE* file) {
    CONTEXT(INFO, "VM File Write");
    for(unsigned int i = 0; i < core->variableCount; i++) {
        fprintf(file, "%s = {0};\n", core->variables[i]);
    }

    fputs("while(true) {\n", file);

    for(unsigned int i = 0; i < core->loopVariableCount; i++) {
        fprintf(file, "%s = {0};\n", core->loopVariables[i]);
    }

    for(unsigned int i = 0; i < core->headBitCount; i++) {
        outputCommand(core, file, core->headBits[i]);
    }

    fputs("switch(opcode) {\n", file);

    for(unsigned int i = 0; i < core->opcodeCount; i++) {
        GenOpCode* code = &core->opcodes[i];
        if(!code->isValid) {
            continue;
        }
        DEBUG("Outputting code %u = %.*s", code->id, code->nameLen, code->name);
        fprintf(file, "// %.*s\ncase %u:\n", code->nameLen, code->name, code->id);
        for(unsigned int j = 0; j < code->lineCount; j++) {
            GenOpCodeLine* line = code->lines[j];
            if(line->hasCondition) {
                fputs("if((conditions >> currentCondition)&1) {\n", file);
                for(unsigned int k = 0; k < line->highBitCount; k++) {
                    outputCommand(core, file, line->highBits[k]);
                }
                fputs("} else {\n", file);
                for(unsigned int k = 0; k < line->lowBitCount; k++) {
                    outputCommand(core, file, line->lowBits[k]);
                }
                fputs("}\n", file);
            } else {
                for(unsigned int k = 0; k < line->lowBitCount; k++) {
                    outputCommand(core, file, line->lowBits[k]);
                }
            }
        }
        fputs("break;\n", file);
    }

    fputs("default: exit(0);\n", file);

    fputs("}\nIP++;\n}}\n", file);
}

void coreCodegen(VMCoreGen* core, const char* filename) {
    CONTEXT(INFO, "Running codegen");
    FILE* file = fopen(filename, "w");

    for(unsigned int i = 0; i < core->headers.entryCapacity; i++) {
        Entry2* entry = &core->headers.entrys[i];
        const char* header = entry->key.key;
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
