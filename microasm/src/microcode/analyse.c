#include "shared/table.h"
#include "shared/memory.h"
#include "shared/graph.h"
#include "emulator/compiletime/create.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/analyse.h"
#include "microcode/parser.h"
#include "microcode/error.h"

// TODO: Improve error messages

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

static Error errNoInput = {0};
static Error errInputWidth = {0};
static Error errInputRedeclare = {0};
static Error errRequiredInputType = {0};
static Error errMissingOpsize = {0};
static Error errMissingPhase = {0};
static void AnalyseInputErrors() {
    newErrEnd(&errNoInput, ERROR_SEMANTIC, "Could not detect input block in microcode.");
    newErrAt(&errInputWidth, ERROR_SEMANTIC, "Input width has to be one or greater");
    newErrAt(&errInputRedeclare, ERROR_SEMANTIC, "Cannot re-declare identifier as input value");
    newErrNoteAt(&errInputRedeclare, "Previously declared here");
    newErrAt(&errRequiredInputType, ERROR_SEMANTIC, "The '%s' identifier must be an input");
    newErrAt(&errMissingOpsize, ERROR_SEMANTIC, "Input statements require an 'opsize' parameter");
    newErrAt(&errMissingPhase, ERROR_SEMANTIC, "Input statements require a 'phase' parameter");
}

static void AnalyseInput(Parser* parser, VMCoreGen* core) {
    AST* mcode = parser->ast;

    if(!mcode->inp.isPresent) {
        error(parser, &errNoInput);
    }

    if(!mcode->inp.isValid) {
        return;
    }

    int totalWidth = 0;
    for(unsigned int i = 0; i < mcode->inp.valueCount; i++) {
        InputValue* val = &mcode->inp.values[i];
        if(val->value.data.value < 1) {
            error(parser, &errInputWidth, &val->value);
        }
        totalWidth += val->value.data.value;

        void* v;
        if(tableGetKey(&identifiers, &val->name, &v)) {
            // existing key
            error(parser, &errInputRedeclare, &val->name, v);
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
            error(parser, &errRequiredInputType, v, "opsize");
        }
        core->opcodeCount = 1 << val->data->data.value;
    } else {
        error(parser, &errMissingOpsize, &mcode->inp.inputHeadToken);
    }

    Token phase = createStrToken("phase");
    if(tableGet(&identifiers, &phase, (void**)&val)) {
        if(val->type != TYPE_INPUT) {
            void* v;
            tableGetKey(&identifiers, &phase, &v);
            error(parser, &errRequiredInputType, v, "opsize");
        }
    } else {
        error(parser, &errMissingPhase, &mcode->inp.inputHeadToken);
    }
}

static Error errNoOrdering = {0};
static Error errBusRead = {0};
static void analyseLineErrors() {
    newErrAt(&errNoOrdering, ERROR_SEMANTIC, "Unable to order microcode bits in line %u");
    newErrAt(&errBusRead, ERROR_SEMANTIC, "Command reads from bus before it was written in line %u");
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
        error(mcode, &errNoOrdering, opcodeName, lineNumber);
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
                error(mcode, &errBusRead, opcodeName, lineNumber);
            }
        }

        for(unsigned int j = 0; j < command->writesLength; j++) {
            Component* bus = &core->components[command->writes[j]];
            bus->busStatus = true;
        }
    }

    return nodes;
}

static Error errNoHeader = {0};
static Error errHeaderLineCount = {0};
static Error errHeaderWrongType = {0};
static Error errIdentifierNotDefined = {0};
static void AnalyseHeaderErrors() {
    newErrEnd(&errNoHeader, ERROR_SEMANTIC, "Could not detect header block in microcode.");
    newErrAt(&errHeaderLineCount, ERROR_SEMANTIC, "Number of lines in header (%u) is too high, the maximum is %u");
    newErrAt(&errHeaderWrongType, ERROR_SEMANTIC, "Cannot use non output bit in header statement");
    newErrNoteAt(&errHeaderWrongType, "Previously declared here");
    newErrAt(&errIdentifierNotDefined, ERROR_SEMANTIC, "Identifier was not defined");
}

static void AnalyseHeader(Parser* parser, VMCoreGen* core) {
    AST* mcode = parser->ast;

    if(!mcode->head.isPresent) {
        error(parser, &errNoHeader);
    }

    if(!mcode->head.isValid) {
        return;
    }

    Identifier* val;
    Token phase = createStrToken("phase");
    if(!tableGet(&identifiers, &phase, (void**)&val)) {
        return;
    }
    unsigned int maxLines = 1 << val->data->data.value;

    if(mcode->head.lineCount > maxLines) {
        error(parser, &errHeaderLineCount, &mcode->head.errorPoint,
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
                    error(parser, &errHeaderWrongType, bit, v);
                }
            } else {
                error(parser, &errIdentifierNotDefined, bit);
            }
        }

        NodeArray nodes = analyseLine(core, parser, line, &mcode->head.errorPoint, i);
        for(unsigned int j = 0; j < nodes.nodeCount; j++) {
            core->headBits[bitCounter] = nodes.nodes[j]->value;
            bitCounter++;
        }
    }
}

static Error errOpcodeIdSize = {0};
static Error errOpcodeLineCount = {0};
static Error errOpcodeWrongType = {0};
static void AnalyseOpcodeErrors() {
    newErrAt(&errOpcodeIdSize, ERROR_SEMANTIC, "Opcode id is too large");
    newErrAt(&errOpcodeLineCount, ERROR_SEMANTIC, "Number of lines in opcode is too high");
    newErrAt(&errOpcodeWrongType, ERROR_SEMANTIC, "Opcode bits must be outputs");
    newErrNoteAt(&errOpcodeWrongType, "Previously declared here");
}

static void AnalyseOpcode(Parser* parser, VMCoreGen* core) {
    AST* mcode = parser->ast;

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
            error(parser, &errOpcodeIdSize, &code->id);
        }

        if(code->lineCount > maxLines) {
            error(parser, &errOpcodeLineCount, &code->name);
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
                        error(parser, &errOpcodeWrongType, bit, v);
                    }
                } else {
                    error(parser, &errIdentifierNotDefined, bit);
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

static bool errorsInitialised;
typedef void (*errorInitialiser)();
static errorInitialiser errorInitialisers[] = {
    AnalyseInputErrors,
    analyseLineErrors,
    AnalyseHeaderErrors,
    AnalyseOpcodeErrors
};

void InitAnalysis() {
    if(!errorsInitialised) {
        for(unsigned int i = 0; i < sizeof(errorInitialisers)/sizeof(errorInitialiser); i++) {
            errorInitialisers[i]();
        }
    }
}

static Analysis Analyses[] = {
    AnalyseInput,
    AnalyseHeader,
    AnalyseOpcode,
};

void Analyse(Parser* parser, VMCoreGen* core) {
    if(parser->hadError)return;
    if(!errorsInitialised)InitAnalysis();

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
