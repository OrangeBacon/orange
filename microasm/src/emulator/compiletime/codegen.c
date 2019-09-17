#include "emulator/compiletime/codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include "shared/graph.h"
#include "microcode/error.h"

static NodeArray analyseLine(VMCoreGen* core, Parser* mcode, BitArray* line, Token* opcodeName, unsigned int lineNumber) {
    Graph graph;
    InitGraph(&graph);

    Table outputMap;
    initTable(&outputMap, tokenHash, tokenCmp);

    for(unsigned int i = 0; i < core->commandCount; i++) {
        Token* key = createStrTokenPtr(core->commands[i].name);
        tableSet(&outputMap, key, createUIntTokenPtr(i));
    }

    for(unsigned int i = 0; i < line->dataCount; i++) {
        Token* value;
        tableGet(&outputMap, &line->datas[i], (void**)&value);
        unsigned int command = value->data.value;
        AddNode(&graph, command);
        for(unsigned int j = 0; j < core->commands[command].changesLength; j++) {
            unsigned int changed = core->commands[command].changes[j];
            for(unsigned int k = 0; k < line->dataCount; k++) {
                Token* value;
                tableGet(&outputMap, &line->datas[k], (void**)&value);
                unsigned int comm = value->data.value;
                for(unsigned int l = 0; l < core->commands[comm].dependsLength; l++) {
                    unsigned int depended = core->commands[comm].depends[l];
                    if(changed == depended) {
                        AddEdge(&graph, command, comm);
                    }
                }
            }
        }
    }

    NodeArray nodes = TopologicalSort(&graph);
    if(!nodes.validArray) {
        warnAt(mcode, 200, opcodeName, "Unable to order microcode bits in line %u", lineNumber);
    }

    return nodes;
}

void coreLinkAnalysisResult(VMCoreGen* core, Parser* mcode, AnalysisAst* ast) {
    addHeader(core, "<stdbool.h>");
    addHeader(core, "<stdlib.h>");

    core->opcodeCount = 1 << ast->opsize;
    core->opcodes = ArenaAlloc(sizeof(GenOpCode) * core->opcodeCount);
    for(unsigned int i = 0; i < core->opcodeCount; i++) {
        core->opcodes[i].isValid = false;
    }

    for(unsigned int i = 0; i < mcode->ast.opcodeCount; i++) {
        OpCode* code = &mcode->ast.opcodes[i];
        GenOpCode* gencode = &core->opcodes[code->id.data.value];

        gencode->isValid = true;
        gencode->name = TOKEN_GET(code->name);
        gencode->nameLen = code->name.length;

        unsigned int count = 0;
        for(unsigned int j = 0; j < code->lineCount; j++) {
            Line** line = &code->lines[j];
            count += (*line)->bits.dataCount;
        }
        gencode->bitCount = count;
        gencode->bits = ArenaAlloc(sizeof(unsigned int) * gencode->bitCount);

        unsigned int bitCounter = 0;
        for(unsigned int j = 0; j < code->lineCount; j++) {
            BitArray* line = &code->lines[j]->bits;

            NodeArray nodes = analyseLine(core, mcode, line, &code->name, j);

            for(unsigned int k = 0; k < nodes.nodeCount; k++) {
                gencode->bits[bitCounter] = nodes.nodes[k]->value;
                bitCounter++;
            }
        }
    }

    unsigned int count = 0;
    for(unsigned int j = 0; j < mcode->ast.head.lineCount; j++) {
        BitArray* line = &mcode->ast.head.lines[j];
        count += line->dataCount;
    }

    core->headCount = count;
    core->headBits = ArenaAlloc(sizeof(unsigned int) * count);

    unsigned int bitCounter = 0;
    for(unsigned int i = 0; i < mcode->ast.head.lineCount; i++) {
        BitArray* line = &mcode->ast.head.lines[i];
        
        NodeArray nodes = analyseLine(core, mcode, line, &mcode->ast.head.errorPoint, i);

        for(unsigned int j = 0; j < nodes.nodeCount; j++) {
            core->headBits[bitCounter] = nodes.nodes[j]->value;
            bitCounter++;
        }
    }

    if(mcode->hadError) {
        exit(-1);
    }
}

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