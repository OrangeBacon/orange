#include "token.h"
#include "ast.h"
#include "analyse.h"
#include "parser.h"
#include "error.h"
#include "table.h"
#include "memory.h"

typedef void(*Analysis)(Parser* parser);

typedef enum IdentifierType {
    TYPE_INPUT,
    TYPE_OUTPUT
} IdentifierType;

typedef struct Identifier {
    IdentifierType type;
    Token* data;
} Identifier;

Table identifiers;

static void AnalyseOutput(Parser* parser) {
    Microcode* mcode = &parser->ast;

    if(mcode->out.width.data.value < 1) {
        errorAt(parser, 100, &mcode->out.width, "Output width has to be one or greater");
    }

    Table outputs;
    initTable(&outputs, tokenHash, tokenCmp);

    for(unsigned int i = 0; i < mcode->out.valueCount; i++) {
        OutputValue* val = &mcode->out.values[i];
        void* v;
        if(tableGetKey(&outputs, &val->id, &v)) {
            // existing key
            warnAt(parser, 103, &val->id, "Cannot re-declare output id");
            noteAt(parser, v, "Previously declared here");
            continue;
        }
        tableSet(&outputs, &val->id, &val->name);

        if(tableGetKey(&identifiers, &val->name, &v)) {
            warnAt(parser, 104, &val->name, "Cannot identifier as output");
            noteAt(parser, v, "Previously declared here");
        }
        
        Identifier* id = ArenaAlloc(sizeof(Identifier));
        id->type = TYPE_OUTPUT;
        id->data = &val->id;
        tableSet(&identifiers, &val->name, id);
    }
}

static void AnalyseInput(Parser* parser) {
    Microcode* mcode = &parser->ast;

    for(unsigned int i = 0; i < mcode->inp.valueCount; i++) {
        InputValue* val = &mcode->inp.values[i];
        if(val->value.data.value < 1) {
            errorAt(parser, 101, &val->value, "Input width has to be one or greater");
        }

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
    Token opsize;
    opsize.base = "opsize";
    opsize.length = 6;
    opsize.offset = 0;
    if(tableGet(&identifiers, &opsize, (void**)&val)) {
        if(val->type != TYPE_INPUT) {
            void* v;
            tableGetKey(&identifiers, &opsize, &v);
            warnAt(parser, 107, v, "The 'opsize' identifier must be an input");
        }
    } else {
        warnAt(parser, 108, &mcode->inp.inputHeadToken, "Input statements require an 'opsize' parameter");
    }

    Token phase;
    phase.base = "phase";
    phase.length = 5;
    phase.offset = 0;
    if(tableGet(&identifiers, &phase, (void**)&val)) {
        if(val->type != TYPE_INPUT) {
            void* v;
            tableGetKey(&identifiers, &phase, &v);
            warnAt(parser, 107, v, "The 'phase' identifier must be an input");
        }
    } else {
        warnAt(parser, 108, &mcode->inp.inputHeadToken, "Input statements require a 'phase' parameter");
    }
}

static void AnalyseHeader(Parser* parser) {
    Microcode* mcode = &parser->ast;

    for(unsigned int i = 0; i < mcode->head.bitCount; i++) {
        Token* bit = &mcode->head.bits[i];

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

static Analysis Analyses[] = {
    AnalyseInput,
    AnalyseOutput,
    AnalyseHeader
};

void Analyse(Parser* parser) {
    initTable(&identifiers, tokenHash, tokenCmp);
    for(unsigned int i = 0; i < sizeof(Analyses)/sizeof(Analysis); i++) {
        Analyses[i](parser);
    }
}