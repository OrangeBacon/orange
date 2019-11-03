#include "microcode/analyse.h"

#include <string.h>
#include "shared/table.h"
#include "shared/memory.h"
#include "shared/graph.h"
#include "emulator/compiletime/create.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/parser.h"
#include "microcode/error.h"

// TODO fix opcode ids so they are correct between ast and vmcoregen
// TODO add analysis for types, bitgroups, parameterised microcode bits

typedef struct IdentifierParameter {
    unsigned int value;
    Token* definition;
} IdentifierParameter;

typedef struct IdentifierControlBit {
    unsigned int value;
} IdentifierControlBit;

typedef enum IdentifierType {
    TYPE_PARAMETER,
    TYPE_VM_CONTROL_BIT
} IdentifierType;

typedef struct Identifier {
    IdentifierType type;
    union {
        IdentifierParameter parameter;
        IdentifierControlBit control;
    } as;
} Identifier;

Table identifiers;

static Identifier* getIdentifier(char* name) {
    Identifier* value;
    if(!tableGet(&identifiers, name, (void**)&value)) {
        return NULL;
    }
    return value;
}

static Error errDuplicateParameter = {0};
static Error errParameterIsControl;
static void analyseParameterErrors() {
    newErrAt(&errDuplicateParameter, ERROR_SEMANTIC, "Duplicate value for parameter found");
    newErrNoteAt(&errDuplicateParameter, "Originally defined here");
    newErrAt(&errParameterIsControl, ERROR_SEMANTIC,
        "Parameter name is defined as the name for a control bit");
}

static bool foundPhase = false;
static bool foundOpsize = false;
static void analyseParameter(Parser* parser, ASTStatement* s) {
    char* key = (char*)s->as.parameter.name.data.string;

    Identifier* current;
    if(tableGet(&identifiers, key, (void**)&current)) {
        switch(current->type) {
            case TYPE_PARAMETER:
                error(parser, &errDuplicateParameter, 
                    &s->as.parameter.name, current->as.parameter.definition);
                break;
            case TYPE_VM_CONTROL_BIT:
                error(parser, &errParameterIsControl, &s->as.parameter.name);
        }
    }

    Identifier* value = ArenaAlloc(sizeof(Identifier));
    value->type = TYPE_PARAMETER;
    value->as.parameter.definition = &s->as.parameter.name;
    value->as.parameter.value = s->as.parameter.value.data.value;
    tableSet(&identifiers, key, (void*)value);

    if(strcmp(key, "phase") == 0) {
        foundPhase = true;
        return;
    }

    if(strcmp(key, "opsize") == 0) {
        foundOpsize = true;
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
        tableGet(&identifiers, (char*)line->datas[i].data.data.string, (void**)&value);
        unsigned int command = value->as.control.value;
        AddNode(&graph, command);
        for(unsigned int j = 0; j < core->commands[command].changesLength; j++) {
            unsigned int changed = core->commands[command].changes[j];
            for(unsigned int k = 0; k < line->dataCount; k++) {
                Identifier* value;
                tableGet(&identifiers, (char*)line->datas[k].data.data.string, (void**)&value);
                unsigned int comm = value->as.control.value;
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

static Error errIdentifierNotDefined = {0};
static Error errMCodeBitIsParameter = {0};
static void mcodeBitArrayCheckErrors() {
    newErrAt(&errIdentifierNotDefined, ERROR_SEMANTIC, "Identifier was not defined");
    newErrAt(&errMCodeBitIsParameter, ERROR_SEMANTIC, 
        "Identifier previously defined as a parameter, control bit required");
    newErrNoteAt(&errMCodeBitIsParameter, "Defined here");
}

static bool mcodeBitArrayCheck(Parser* parser, BitArray* arr) {
    bool passed = true;

    for(unsigned int j = 0; j < arr->dataCount; j++) {
        Token* bit = &arr->datas[j].data;

        Identifier* val;
        if(!tableGet(&identifiers, (char*)bit->data.string, (void**)&val)) {
            error(parser, &errIdentifierNotDefined, bit);
            passed = false;
            continue;
        }
        switch(val->type) {
            case TYPE_VM_CONTROL_BIT: break;
            case TYPE_PARAMETER:
                error(parser, &errMCodeBitIsParameter, 
                    bit, val->as.parameter.definition);
                passed = false;
        }
    }

    return passed;
}

static Error errDuplicateHeader = {0};
static Error errHeaderLineCount = {0};
static Error errHeaderWrongType = {0};
static void analyseHeaderErrors() {
    newErrAt(&errDuplicateHeader, ERROR_SEMANTIC, 
        "Cannot have more than one header statement in a microcode");
    newErrNoteAt(&errDuplicateHeader, "Header first included here");
    newErrAt(&errHeaderLineCount, ERROR_SEMANTIC, 
        "Number of lines in header (%u) is too high, the maximum is %u");
    newErrAt(&errHeaderWrongType, ERROR_SEMANTIC, "Cannot use non output bit in header statement");
    newErrNoteAt(&errHeaderWrongType, "Previously declared here");
}

static bool parsedHeader = false;
static ASTStatement* firstHeader = NULL;
static void analyseHeader(Parser* parser, ASTStatement* s, VMCoreGen* core) {
    if(parsedHeader) {
        error(parser, &errDuplicateHeader, &s->as.header.errorPoint,
            firstHeader->as.header.errorPoint);
    }
    parsedHeader = true;
    firstHeader = s;

    if(!foundPhase) return;
    Identifier* phase = getIdentifier("phase");
    unsigned int maxLines = (1 << phase->as.parameter.value);

    if(s->as.header.lineCount > maxLines) {
        error(parser, &errHeaderLineCount, &s->as.header.errorPoint,
            s->as.header.lineCount, maxLines);
    }

    for(unsigned int i = 0; i < s->as.header.lineCount; i++) {
        BitArray* line = &s->as.header.lines[i];

        if(!mcodeBitArrayCheck(parser, line)) {
            continue;
        }

        NodeArray nodes = analyseLine(core, parser, line, &s->as.header.errorPoint, i);
        for(unsigned int j = 0; j < nodes.nodeCount; j++) {
            PUSH_ARRAY(unsigned int, *core, headBit, nodes.nodes[j]->value);
        }
    }
}

static Error errOpcodeIdSize = {0};
static Error errOpcodeLineCount = {0};
static Error errOpcodeWrongType = {0};
static void analyseOpcodeErrors() {
    newErrAt(&errOpcodeIdSize, ERROR_SEMANTIC, "Opcode id is too large");
    newErrAt(&errOpcodeLineCount, ERROR_SEMANTIC, "Number of lines in opcode is too high");
    newErrAt(&errOpcodeWrongType, ERROR_SEMANTIC, "Opcode bits must be outputs");
    newErrNoteAt(&errOpcodeWrongType, "Previously declared here");
}

static void analyseOpcode(Parser* parser, ASTStatement* s, VMCoreGen* core) {
    if(!foundPhase) return;
    if(!parsedHeader) return;
    Identifier* phase = getIdentifier("phase");
    unsigned int maxLines = (1 << phase->as.parameter.value) - 
        firstHeader->as.parameter.value.data.value;

    if(!foundOpsize) return;
    Identifier* opsize = getIdentifier("opsize");
    unsigned int maxOpCodes = (1 << opsize->as.parameter.value);

    GenOpCode gencode;
    ARRAY_ALLOC(GenOpCodeLine*, gencode, line);

    if(s->as.opcode.id.data.value >= maxOpCodes) {
        error(parser, &errOpcodeIdSize, &s->as.opcode.id);
    }

    if(s->as.opcode.lineCount > maxLines) {
        error(parser, &errOpcodeLineCount, &s->as.opcode.name);
    }

    gencode.id = s->as.opcode.id.data.value;
    gencode.name = s->as.opcode.name.start;
    gencode.nameLen = s->as.opcode.name.length;

    for(unsigned int j = 0; j < s->as.opcode.lineCount; j++) {
        Line* line = s->as.opcode.lines[j];
        GenOpCodeLine* genline = ArenaAlloc(sizeof(GenOpCodeLine));
        genline->hasCondition = line->hasCondition;

        if(!mcodeBitArrayCheck(parser, &line->bitsLow)) {
            continue;
        }
        NodeArray lowNodes = analyseLine(core, parser, &line->bitsLow, &s->as.opcode.name, j);
        ARRAY_ALLOC(unsigned int, *genline, lowBit);
        for(unsigned int k = 0; k < lowNodes.nodeCount; k++) {
            PUSH_ARRAY(unsigned int, *genline, lowBit, lowNodes.nodes[k]->value);
        }

        if(line->hasCondition) {
            if(!mcodeBitArrayCheck(parser, &line->bitsHigh)) {
                continue;
            }
            NodeArray highNodes = analyseLine(core, parser, &line->bitsHigh, &s->as.opcode.name, j);
            ARRAY_ALLOC(unsigned int, *genline, highBit);
            for(unsigned int k = 0; k < highNodes.nodeCount; k++) {
                PUSH_ARRAY(unsigned int, *genline, highBit, highNodes.nodes[k]->value);
            }
        } else {
            genline->highBitCapacity = genline->lowBitCapacity;
            genline->highBitCount = genline->lowBitCount;
            genline->highBits = genline->lowBits;
        }
        PUSH_ARRAY(GenOpCodeLine*, gencode, line, genline);
    }

    PUSH_ARRAY(GenOpCode, *core, opcode, gencode);
}

static Error errNoHeader = {0};
static Error errNoOpsize = {0};
static Error errNoPhase = {0};
static void analyseCheckParsedErrors() {
    newErrEnd(&errNoHeader, ERROR_SEMANTIC, "A header statement is required, but not found");
    newErrEnd(&errNoOpsize, ERROR_SEMANTIC, "An opsize parameter is required, but not found");
    newErrEnd(&errNoPhase, ERROR_SEMANTIC, "A phase parameter is required, but not found");
}

static void analyseCheckParsed(Parser* parser) {
    if(!parsedHeader) {
        error(parser, &errNoHeader);
    }
    if(!foundOpsize) {
        error(parser, &errNoOpsize);
    }
    if(!foundPhase) {
        error(parser, &errNoPhase);
    }
}

static bool errorsInitialised;
typedef void (*errorInitialiser)();
static errorInitialiser errorInitialisers[] = {
    analyseParameterErrors,
    analyseLineErrors,
    mcodeBitArrayCheckErrors,
    analyseHeaderErrors,
    analyseOpcodeErrors,
    analyseCheckParsedErrors
};

void InitAnalysis() {
    if(!errorsInitialised) {
        for(unsigned int i = 0; i < sizeof(errorInitialisers)/sizeof(errorInitialiser); i++) {
            errorInitialisers[i]();
        }
    }
}

void Analyse(Parser* parser, VMCoreGen* core) {
    if(parser->hadError)return;
    if(!errorsInitialised)InitAnalysis();

    initTable(&identifiers, strHash, strCmp);

    for(unsigned int i = 0; i < core->commandCount; i++) {
        char* key = (char*)core->commands[i].name;
        Identifier* value = ArenaAlloc(sizeof(Identifier));
        value->type = TYPE_VM_CONTROL_BIT;
        value->as.control.value = i;
        tableSet(&identifiers, key, (void*)value);
    }

    for(unsigned int i = 0; i < parser->ast->statementCount; i++) {
        ASTStatement* s = &parser->ast->statements[i];
        if(!s->isValid) continue;
        switch(s->type) {
            case AST_BLOCK_PARAMETER: analyseParameter(parser, s); break;
            case AST_BLOCK_HEADER: analyseHeader(parser, s, core); break;
            case AST_BLOCK_OPCODE: analyseOpcode(parser, s, core); break;
            case AST_BLOCK_TYPE: break;
            case AST_BLOCK_BITGROUP: break;
        }
    }

    analyseCheckParsed(parser);
}
