#include "arg.h"

void argInit(argParser* parser) {
    initTable(&parser->modes, strHash, strCmp);
}

argParser* argMode(argParser* parser, const char* name) {
    (void)name;
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