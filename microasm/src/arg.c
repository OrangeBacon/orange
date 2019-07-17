#include <string.h>
#include "arg.h"
#include "platform.h"
#include "memory.h"

static void argInternalError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Implementation Error: %s\n", message);
}

void argArguments(argParser* parser, int argc, char** argv) {
    parser->argc = argc;
    parser->argv = argv;
}

void argInit(argParser* parser) {
    initTable(&parser->modes, strHash, strCmp);
    parser->success = true;
    parser->parsed = false;
    parser->parseOptions = true;
}

argParser* argMode(argParser* parser, const char* name) {
    if(tableHas(&parser->modes, (void*)name)) {
        argInternalError(parser, "Mode already exists");
    }

    argParser* new = ArenaAlloc(sizeof(argParser));
    tableSet(&parser->modes, (void*)name, new);
    return parser;
}

void argString(argParser* parser, const char* name) {
    (void)parser;
    (void)name;
}

void argParse(argParser* parser) {
    if(parser->parsed) {
        return;
    }

    for(int i = 0; i < parser->argc; i++) {
        argParser* new;
        if(tableGet(&parser->modes, parser->argv[i], (void**)&new)) {
            new->parseOptions = parser->parseOptions;
            argArguments(new, parser->argc - 1, &parser->argv[1]);
            argParse(new);
            break;
        }

        if(parser->parseOptions && strlen(parser->argv[i]) == 2 
            && parser->argv[i][0] == '-' && parser->argv[i][1] == '-') {
            parser->parseOptions = false;
        }
    }
    parser->parsed = true;
}

bool argSuccess(argParser* parser) {
    return parser->success;
}