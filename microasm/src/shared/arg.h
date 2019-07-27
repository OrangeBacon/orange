#include "shared/table.h"
#include "shared/memory.h"

// positional argument
// will allow for multiple argument types (ie add int)
typedef struct posArg {
    enum type {
        TYPE_STRING
    } type;
    union value {
        char* as_string;
    } value;
    const char* description;
} posArg;

// argument parser state
typedef struct argParser {
    // arguments to be parsed
    int argc;
    char** argv;
    
    // map of all avaliable modes to parsers
    // that can handle the mode
    Table modes;

    // has the parser run
    bool parsed;

    // has the parser currently encountered any errors
    bool success;

    // should options be parsed
    bool parseOptions;

    // has this mode been taken
    bool modeTaken;

    // was another parser invoked
    bool usedSubParser;

    // positional arguments required
    DEFINE_ARRAY(posArg, posArg);

    // which positional argument is next to be parsed
    unsigned int currentPosArg;

    const char* name;
    unsigned int nameLength;
} argParser;

// setup a new argument parser
void argInit(argParser* parser, const char* name);

// create a mode for the parser (sub-command)
// that can handle parsing for that sub-command
// and enable the mode to be detected
argParser* argMode(argParser* parser, const char* name);

// add a string required positional argument to a parser
void argString(argParser* parser, const char* name);

// run the parser on the arguments prieviously set
void argParse(argParser* parser);

// set the parser's arguments to one beyond the provided
// argc and argc, for ignoring program name and mode name
void argArguments(argParser* parser, int argc, char** argv);

// has the parser !(encountered any errors)
bool argSuccess(argParser* parser);

#define strArg(parser, count) parser.posArgs[count].value.as_string