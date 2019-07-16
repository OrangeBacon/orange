#include "table.h"

typedef struct argParser {
    int argc;
    char** argv;
    Table modes;
    bool parsed;
    bool success;
} argParser;

void argInit(argParser* parser);
argParser* argMode(argParser* parser, const char* name);
void argString(argParser* parser, const char* name);
void argParse(argParser* parser, int argc, char** argv);
bool argSuccess(argParser* parser);