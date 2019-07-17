#include <string.h>
#include "arg.h"
#include "platform.h"

static void argInternalError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Implementation Error: %s\n", message);
}

static void argError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Error: %s\n", message);
}

void argArguments(argParser* parser, int argc, char** argv) {
    parser->argc = argc - 1;
    parser->argv = &argv[1];
}

void argInit(argParser* parser) {
    initTable(&parser->modes, strHash, strCmp);
    ARRAY_ALLOC(posArg, *parser, posArg);
    parser->success = true;
    parser->parsed = false;
    parser->parseOptions = true;
    parser->usedSubParser = false;
    parser->modeTaken = false;
    parser->currentPosArg = 0;
}

argParser* argMode(argParser* parser, const char* name) {
    if(tableHas(&parser->modes, (void*)name)) {
        argInternalError(parser, "Mode already exists");
    }

    argParser* new = ArenaAlloc(sizeof(argParser));
    argInit(new);
    tableSet(&parser->modes, (void*)name, new);
    return new;
}

void argString(argParser* parser, const char* name) {
    posArg arg;
    arg.description = name;
    arg.type = TYPE_STRING;
    PUSH_ARRAY(posArg, *parser, posArg, arg);
}

void argParse(argParser* parser) {
    if(parser->parsed) {
        return;
    }
    parser->modeTaken = true;
    for(int i = 0; i < parser->argc; i++) {
        argParser* new;
        if(tableGet(&parser->modes, parser->argv[i], (void**)&new)) {
            new->parseOptions = parser->parseOptions;
            argArguments(new, parser->argc, parser->argv);
            argParse(new);
            parser->success &= new->success;
            parser->usedSubParser = true;
            break;
        }

        if(parser->parseOptions && strlen(parser->argv[i]) == 2 
            && parser->argv[i][0] == '-' && parser->argv[i][1] == '-') {
            parser->parseOptions = false;
            continue;
        }

        if(parser->currentPosArg < parser->posArgCount) {
            posArg* arg = &parser->posArgs[parser->currentPosArg];
            switch(arg->type) {
                case TYPE_STRING:
                    arg->value.as_string = parser->argv[i];
            }
            parser->currentPosArg++;
            continue;
        }

        argError(parser, "unexpected argument");
        return;
    }

    if(!parser->usedSubParser && parser->currentPosArg != parser->posArgCount) {
        argError(parser, "not enough arguments");
    }
    parser->parsed = true;
}

bool argSuccess(argParser* parser) {
    return parser->success;
}