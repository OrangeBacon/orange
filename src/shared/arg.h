#ifndef ARG_H
#define ARG_H

#include "shared/table2.h"
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
    const char* helpMessage;
} posArg;

typedef struct argParser argParser;

// data for a optional, non-positional argument
typedef struct optionArg {
    // multi character name
    const char* longName;

    // single character name
    bool hasShortName;
    char shortName;

    enum optType {
        OPT_STRING,
        OPT_INT,
        OPT_NO_ARG
    } type;
    union optValue {
        char* as_string;
        intmax_t as_int;
    } value;

    // name of argument to this option if string option
    const char* argumentName;

    // was the argument present
    bool found;

    const char* helpMessage;

    bool isUniversal;
    argParser* universalRoot;
    bool universalChildrenOnly;
    bool printed;

} optionArg;

// argument parser state
struct argParser {
    // arguments to be parsed
    int argc;
    char** argv;

    // map of all avaliable modes to parsers
    // that can handle the mode
    Table2 modes;

    // map of all optional arguments
    // by their long names
    Table2 options;

    // has the parser run
    bool parsed;

    // has the parser currently encountered any errors
    bool success;

    // should options be parsed
    bool parseOptions;

    // was another parser invoked
    bool usedSubParser;

    // positional arguments required
    ARRAY_DEFINE(posArg, posArg);

    // which positional argument is next to be parsed
    unsigned int currentPosArg;

    // the name of the current parser, including executable name (and mode name(s))
    const char* name;

    ARRAY_DEFINE(const char*, errorMessage);

    bool isSubParser;

    optionArg* helpOption;
    optionArg* versionOption;

    argParser* errorRoot;

    const char* helpMessage;
    const char* versionString;

    bool printUsage;

    ARRAY_DEFINE(optionArg*, universalOption);
};

// setup a new argument parser
void argInit(argParser* parser, const char* name);

void argPrintMessage(argParser* parser);

// create a mode for the parser (sub-command)
// that can handle parsing for that sub-command
// and enable the mode to be detected
argParser* argMode(argParser* parser, const char* name);

// add a string required positional argument to a parser
posArg* argString(argParser* parser, const char* name);

void argSetHelpMode(argParser* parser, char shortName, const char* longName);

// run the parser on the arguments prieviously set
// can call exit() under some circumstances
void argParse(argParser* parser);

// set the parser's arguments to one beyond the provided
// argc and argc, for ignoring program name and mode name
void argArguments(argParser* parser, int argc, char** argv);

// add an optional argument to the parser, can be identified by "-${shortName}"
// or "--${longName}".  The argument cannot be repeated and optionaly takes an argument
optionArg* argOption(argParser* parser, char shortName, const char* longName);
optionArg* argOptionString(argParser* parser, char shortName, const char* longName);
optionArg* argOptionInt(argParser* parser, char shortName, const char* longName);

optionArg* argUniversalOption(argParser* parser, char shortName, const char* longName, bool childrenOnly);
optionArg* argUniversalOptionString(argParser* parser, char shortName, const char* longName, bool childrenOnly);
optionArg* argUniversalOptionInt(argParser* parser, char shortName, const char* longName, bool childrenOnly);

void argAddExistingOption(argParser* parser, optionArg* arg);

// has the parser !(encountered any errors)
bool argSuccess(argParser* parser);

// get the count'th argument of a parser as a char pointer
#define strArg(parser, count) (parser).posArgs[count].value.as_string

#endif
