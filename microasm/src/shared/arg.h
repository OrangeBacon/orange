#ifndef ARG_H
#define ARG_H

#include "shared/table.h"
#include "shared/memory.h"

// positional argument
// will allow for multiple argument types (ie add int)
typedef struct posArg {
    enum posType {
        POS_STRING
    } type;

    union posValue {
        char* as_string;
    } value;

    const char* description;
} posArg;

// the function run in an action option.
// void* ctx is a user provided value from calling optActionArg
// return true = parser should continue
// return false = exits after calling function
typedef bool (*optionAction)(void* ctx);

// data for a optional, non-positional argument
typedef struct optionArg {
    // multi character name
    const char* longName;

    // single character name
    bool hasShortName;
    char shortName;

    enum optType {
        OPT_STRING,
        OPT_ACTION,
        OPT_NONE
    } type;
    union optValue {
        char* as_string;
    } value;

    // name of argument to this option if string option
    const char* argumentName;

    // action if the argument is an action argument
    optionAction action;
    void* ctx;

    // was the argument present
    bool found;
} optionArg;

// argument parser state
typedef struct argParser {
    // arguments to be parsed
    int argc;
    char** argv;
    
    // map of all avaliable modes to parsers
    // that can handle the mode
    Table modes;

    // map of all optional arguments
    // by their long names
    Table options;

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

    // the name of the current parser, including executable name (and mode name(s))
    const char* name;

    // length of the name
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
// can call exit() under some circumstances
void argParse(argParser* parser);

// set the parser's arguments to one beyond the provided
// argc and argc, for ignoring program name and mode name
void argArguments(argParser* parser, int argc, char** argv);

// add an optional argument to the parser, can be identified by "-${shortName}"
// or "--${longName}".  The argument cannot be repeated and optionaly takes an argument
optionArg* argOption(argParser* parser, char shortName, const char* longName, bool takesArg);

// add an optional argument that runs a function if it is found.
// the option cannot take any arguments
optionArg* argActionOption(argParser* parser, char shortName, const char* longName, optionAction action, void* ctx);

// has the parser !(encountered any errors)
bool argSuccess(argParser* parser);

// get the count'th argument of a parser as a char pointer
#define strArg(parser, count) (parser).posArgs[count].value.as_string

#endif
