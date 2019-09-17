#include "shared/table.h"
#include "shared/memory.h"
#include "emulator/compiletime/create.h"
#include "microcode/token.h"
#include "microcode/ast.h"
#include "microcode/analyse.h"
#include "microcode/parser.h"
#include "microcode/error.h"

typedef void(*Analysis)(Parser* parser, AnalysisAst* out);

typedef enum IdentifierType {
    TYPE_INPUT,
    TYPE_OUTPUT
} IdentifierType;

typedef struct Identifier {
    Token* data;
    IdentifierType type;
} Identifier;

Table identifiers;

static void AnalyseInput(Parser* parser, AnalysisAst* out) {
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
        out->opsize = val->data->data.value;
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

static void AnalyseHeader(Parser* parser, AnalysisAst* out) {
    (void)out;
    AST* mcode = &parser->ast;

    if(!mcode->head.isValid) {
        return;
    }

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
    }
}

static void AnalyseOpcode(Parser* parser, AnalysisAst* out) {
    AST* mcode = &parser->ast;

    Table parameters;
    initTable(&parameters, tokenHash, tokenCmp);
    tableSet(&parameters, (void*)createStrTokenPtr("Reg"), (void*)true);

    for(unsigned int i = 0; i < mcode->opcodeCount; i++) {
        OpCode* code = &mcode->opcodes[i];

        if(!code->isValid) {
            continue;
        }
        
        if(code->id.data.value >= (unsigned int)(2 << (out->opsize - 1))) {
            warnAt(parser, 109, &code->id, "Opcode id is too large");
        }

        for(unsigned int j = 0; j < code->lineCount; j++) {
            Line* line = code->lines[j];

            for(unsigned int k = 0; k < line->bits.dataCount; k++) {
                Token* bit = &line->bits.datas[k];
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

            for(unsigned int k = 0; k < line->conditionCount; k++) {
                Condition* cond = &line->conditions[k];
                if(!(cond->value.data.value == 0 || cond->value.data.value == 1)) {
                    warnAt(parser, 112, &cond->value, "Condition values must be 0 or 1");
                }
            }
        }

        for(unsigned int i = 0; i < code->parameterCount; i++) {
            Token* param = &code->parameters[i];
            void* _;
            if(!tableGet(&parameters, param, &_)) {
                warnAt(parser, 115, param, "Undefined parameter type");
            }
        }
    }
}

static Analysis Analyses[] = {
    AnalyseInput,
    AnalyseHeader,
    AnalyseOpcode
};

AnalysisAst* Analyse(Parser* parser, VMCoreGen* core) {
    initTable(&identifiers, tokenHash, tokenCmp);

    for(unsigned int i = 0; i < core->commandCount; i++) {
        Token* key = createStrTokenPtr(core->commands[i].name);
        tableSet(&identifiers, key, &(Identifier) {
            .data = createUIntTokenPtr(i),
            .type = TYPE_OUTPUT
        });
    }

    AnalysisAst* out = ArenaAlloc(sizeof(AnalysisAst));

    for(unsigned int i = 0; i < sizeof(Analyses)/sizeof(Analysis); i++) {
        Analyses[i](parser, out);
    }

    return out;
}