#include "table.h"
#include "memory.h"

typedef struct posArg {
    enum type {
        TYPE_STRING
    } type;
    union value {
        char* as_string;
    } value;
    const char* description;
} posArg;

typedef struct argParser {
    int argc;
    char** argv;
    Table modes;
    bool parsed;
    bool success;
    bool parseOptions;
    bool modeTaken;
    bool usedSubParser;
    DEFINE_ARRAY(posArg, posArg);
    unsigned int currentPosArg;
} argParser;

void argInit(argParser* parser);
argParser* argMode(argParser* parser, const char* name);
void argString(argParser* parser, const char* name);
void argParse(argParser* parser);
void argArguments(argParser* parser, int argc, char** argv);
bool argSuccess(argParser* parser);

#define strArg(parser, count) parser.posArgs[count].value.as_string