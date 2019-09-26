#include "shared/table.h"
#include "shared/memory.h"
#include "shared/graph.h"
#include "emulator/compiletime/create.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/analyse.h"
#include "microcode/parser.h"
#include "microcode/error.h"

typedef void(*Analysis)(Parser* parser, VMCoreGen* core);

typedef enum IdentifierType {
    TYPE_INPUT,
    TYPE_OUTPUT
} IdentifierType;

typedef struct Identifier {
    Token* data;
    IdentifierType type;
} Identifier;

Table identifiers;

static void AnalyseInput(Parser* parser, VMCoreGen* core) {
    AST* mcode = &parser->ast;

    if(!mcode->inp.isValid) {
        return;
    }

    int totalWidth = 0;
    for(unsigned int i = 0; i < mcode->inp.valueCount; i++) {
        InputValue* val = &mcode->inp.values[i];
        if(val->value.data.value < 1) {
            errorAt(parser, 101, &val->value, "Input width has to be one or greater");
        }
        totalWidth += val->value.data.value;

        void* v;
        if(tableGetKey(&identifiers, &val->name, &v)) {
            // existing key
            warnAt(parser, 102, &val->name, "Cannot re-declare identifier as input value");
            noteAt(parser, v, "Previously declared here");
        } else {
            Identifier* id = ArenaAlloc(sizeof(Identifier));
            id->type = TYPE_INPUT;
            id->data = &val->value;
            tableSet(&identifiers, &val->name, id);
        }
    }

    Identifier* val;
    Token opsize = createStrToken("opsize");
    if(tableGet(&identifiers, &opsize, (void**)&val)) {
        if(val->type != TYPE_INPUT) {
            void* v;
            tableGetKey(&identifiers, &opsize, &v);
            warnAt(parser, 107, v, "The 'opsize' identifier must be an input");
        }
        core->opcodeCount = 1 << val->data->data.value;
    } else {
        warnAt(parser, 108, &mcode->inp.inputHeadToken, "Input statements require an 'opsize' parameter");
    }

    Token phase = createStrToken("phase");
    if(tableGet(&identifiers, &phase, (void**)&val)) {
        if(val->type != TYPE_INPUT) {
            void* v;
            tableGetKey(&identifiers, &phase, &v);
            warnAt(parser, 113, v, "The 'phase' identifier must be an input");
        }
    } else {
        warnAt(parser, 114, &mcode->inp.inputHeadToken, "Input statements require a 'phase' parameter");
    }
}

static NodeArray analyseLine(VMCoreGen* core, Parser* mcode, BitArray* line, Token* opcodeName, unsigned int lineNumber) {
    Graph graph;
    InitGraph(&graph);

    for(unsigned int i = 0; i < line->dataCount; i++) {
        Identifier* value;
        if(!tableGet(&identifiers, &line->datas[i], (void**)&value)) {
            break;
        }
        unsigned int command = value->data->data.value;
        AddNode(&graph, command);
        for(unsigned int j = 0; j < core->commands[command].changesLength; j++) {
            unsigned int changed = core->commands[command].changes[j];
            for(unsigned int k = 0; k < line->dataCount; k++) {
                Identifier* value;
                tableGet(&identifiers, &line->datas[k], (void**)&value);
                unsigned int comm = value->data->data.value;
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

    for(unsigned int i = 0; i < core->componentCount; i++) {
        Component* component = &core->components[i];
        component->busStatus = false;
    }

    for(unsigned int i = 0; i < nodes.nodeCount; i++) {
        Command* command = &core->commands[nodes.nodes[i]->value];

        for(unsigned int j = 0; j < command->readsLength; j++) {
            Component* bus = &core->components[command->reads[j]];
            if(!bus->busStatus) {
                warnAt(mcode, 201, opcodeName, "Command reads from bus before it was written "
                    "in line %u", lineNumber);
            }
        }

        for(unsigned int j = 0; j < command->writesLength; j++) {
            Component* bus = &core->components[command->writes[j]];
            bus->busStatus = true;
        }
    }

    return nodes;
}

static void AnalyseHeader(Parser* parser, VMCoreGen* core) {
    AST* mcode = &parser->ast;

    if(!mcode->head.isValid) {
        return;
    }

    Identifier* val;
    Token phase = createStrToken("phase");
    tableGet(&identifiers, &phase, (void**)&val);
    unsigned int maxLines = 1 << val->data->data.value;

    if(mcode->head.lineCount > maxLines) {
        warnAt(parser, 301, &mcode->head.errorPoint, 
            "Number of lines in header (%u) is too high, the maximum is %u",
            mcode->head.lineCount, maxLines);
    }

    unsigned int count = 0;
    for(unsigned int j = 0; j < mcode->head.lineCount; j++) {
        BitArray* line = &mcode->head.lines[j];
        count += line->dataCount;
    }

    core->headCount = count;
    core->headBits = ArenaAlloc(sizeof(unsigned int) * count);

    unsigned int bitCounter = 0;

    for(unsigned int i = 0; i < mcode->head.lineCount; i++) {
        BitArray* line = &mcode->head.lines[i];

        for(unsigned int j = 0; j < line->dataCount; j++) {
            Token* bit = &line->datas[j];

            Identifier* val;
            if(tableGet(&identifiers, bit, (void**)&val)) {
                if(val->type != TYPE_OUTPUT) {
                    void* v;
                    tableGetKey(&identifiers, bit, &v);
                    warnAt(parser, 106, bit, "Cannot use non output bit in header statement");
                    noteAt(parser, v, "Previously declared here");
                }
            } else {
                warnAt(parser, 105, bit, "Identifier was not defined");
            }
        }

        NodeArray nodes = analyseLine(core, parser, line, &mcode->head.errorPoint, i);
        for(unsigned int j = 0; j < nodes.nodeCount; j++) {
            core->headBits[bitCounter] = nodes.nodes[j]->value;
            bitCounter++;
        }
    }
}

static void AnalyseOpcode(Parser* parser, VMCoreGen* core) {
    AST* mcode = &parser->ast;

    Identifier* val;
    Token phase = createStrToken("phase");
    tableGet(&identifiers, &phase, (void**)&val);
    unsigned int maxLines = (1 << val->data->data.value) - mcode->head.lineCount;

    Table parameters;
    initTable(&parameters, tokenHash, tokenCmp);
    tableSet(&parameters, (void*)createStrTokenPtr("Reg"), (void*)true);

    core->opcodes = ArenaAlloc(sizeof(GenOpCode) * core->opcodeCount);
    for(unsigned int i = 0; i < core->opcodeCount; i++) {
        core->opcodes[i].isValid = false;
    }

    for(unsigned int i = 0; i < mcode->opcodeCount; i++) {
        OpCode* code = &mcode->opcodes[i];
        GenOpCode* gencode = &core->opcodes[code->id.data.value];
        ARRAY_ALLOC(GenOpCodeLine*, *gencode, line);

        if(!code->isValid) {
            continue;
        }

        if(code->id.data.value >= (unsigned int)(core->opcodeCount)) {
            warnAt(parser, 109, &code->id, "Opcode id is too large");
        }

        if(code->lineCount > maxLines) {
            warnAt(parser, 302, &code->name, "Number of lines in opcode is too high");
        }

        gencode->isValid = true;
        gencode->name = code->name.start;
        gencode->nameLen = code->name.length;

        for(unsigned int j = 0; j < code->lineCount; j++) {
            Line* line = code->lines[j];
            GenOpCodeLine* genline = ArenaAlloc(sizeof(GenOpCodeLine));
            genline->hasCondition = line->hasCondition;

            for(unsigned int k = 0; k < line->bitsLow.dataCount; k++) {
                Token* bit = &line->bitsLow.datas[k];
                Identifier* bitIdentifier;
                if(tableGet(&identifiers, bit, (void**)&bitIdentifier)) {
                    if(bitIdentifier->type != TYPE_OUTPUT) {
                        void* v;
                        tableGetKey(&identifiers, bit, &v);
                        warnAt(parser, 111, bit, "Opcode bits must be outputs");
                        noteAt(parser, v, "Previously declared here");
                    }
                } else {
                    warnAt(parser, 110, bit, "Identifier is undefined");
                }
            }

            NodeArray lowNodes = analyseLine(core, parser, &line->bitsLow, &code->name, j);
            ARRAY_ALLOC(unsigned int, *genline, lowBit);
            for(unsigned int k = 0; k < lowNodes.nodeCount; k++) {
                PUSH_ARRAY(unsigned int, *genline, lowBit, lowNodes.nodes[k]->value);
            }

            if(line->hasCondition) {
                NodeArray highNodes = analyseLine(core, parser, &line->bitsHigh, &code->name, j);
                ARRAY_ALLOC(unsigned int, *genline, highBit);
                for(unsigned int k = 0; k < highNodes.nodeCount; k++) {
                   PUSH_ARRAY(unsigned int, *genline, highBit, highNodes.nodes[k]->value);
                }
            } else {
                genline->highBitCapacity = genline->lowBitCapacity;
                genline->highBitCount = genline->lowBitCount;
                genline->highBits = genline->lowBits;
            }
            PUSH_ARRAY(GenOpCodeLine*, *gencode, line, genline);
        }
    }
}

static Analysis Analyses[] = {
    AnalyseInput,
    AnalyseHeader,
    AnalyseOpcode
};

void Analyse(Parser* parser, VMCoreGen* core) {
    if(parser->hadError)return;

    initTable(&identifiers, tokenHash, tokenCmp);

    for(unsigned int i = 0; i < core->commandCount; i++) {
        Token* key = createStrTokenPtr(core->commands[i].name);
        Identifier* value = ArenaAlloc(sizeof(Identifier));
        value->data = createUIntTokenPtr(i);
        value->type = TYPE_OUTPUT;
        tableSet(&identifiers, key, value);
    }

    for(unsigned int i = 0; i < sizeof(Analyses)/sizeof(Analysis); i++) {
        Analyses[i](parser, core);
    }
}
