#include "arg.h"
#include "platform.h"

static void argInternalError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Implementation Error: %s\n", message);
}

void argInit(argParser* parser) {
    initTable(&parser->modes, strHash, strCmp);
    parser->success = true;
    parser->parsed = false;
}

argParser* argMode(argParser* parser, const char* name) {
    if(tableHas(&parser->modes, (void*)name)) {
        argInternalError(parser, "Mode already exists");
    }
    tableSet(&parser->modes, (void*)name, parser);
    return parser;
}

void argString(argParser* parser, const char* name) {
    (void)parser;
    (void)name;
}

void argParse(argParser* parser, int argc, char** argv) {
    parser->argc = argc;
    parser->argv = argv;
}

bool argSuccess(argParser* parser) {
    return parser->success;
}