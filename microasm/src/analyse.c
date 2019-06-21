#include "token.h"
#include "ast.h"
#include "analyse.h"
#include "parser.h"
#include "error.h"
#include "table.h"

typedef void(*Analysis)(Parser* parser);

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
        } else {
            tableSet(&outputs, &val->id, &val->name);
        }
    }
}

static void AnalyseInput(Parser* parser) {
    Microcode* mcode = &parser->ast;

    Table inputs;
    initTable(&inputs, tokenHash, tokenCmp);

    for(unsigned int i = 0; i < mcode->inp.valueCount; i++) {
        InputValue* val = &mcode->inp.values[i];
        if(val->value.data.value < 1) {
            errorAt(parser, 101, &val->value, "Input width has to be one or greater");
        }

        void* v;
        if(tableGetKey(&inputs, &val->name, &v)) {
            // existing key
            warnAt(parser, 102, &val->name, "Cannot re-declare input value");
            noteAt(parser, v, "Previously declared here");
        } else {
            tableSet(&inputs, &val->name, &val->value);
        }
    }
}

static Analysis Analyses[] = {
    AnalyseInput,
    AnalyseOutput
};

void Analyse(Parser* parser) {
    for(unsigned int i = 0; i < sizeof(Analyses)/sizeof(Analysis); i++) {
        Analyses[i](parser);
    }
}