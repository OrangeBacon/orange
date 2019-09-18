#include <stdlib.h>
#include <string.h>
#include "shared/arg.h"
#include "shared/platform.h"

// display error caused by in-correct arg-parser description
// ony caused by errors in the executable - when raised
// always a bug
static void argInternalError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Implementation Error: %s\n", message);
    exit(1);
}

// list of characters
typedef struct charArr {
    DEFINE_ARRAY(char, char);
} charArr;

// list of argument parsers
typedef struct argParserArr {
    DEFINE_ARRAY(argParser*, parser);
} argParserArr;

// sorting function for character list
int charSort(const void* a, const void* b) {
    const char* x = a;
    const char* y = b;
    return *x - *y;
}

// sorting function for argument parser list
int parserSort(const void* a, const void* b) {
    const argParser* const * x = a;
    const argParser* const * y = b;
    return strcmp((*x)->name, (*y)->name);
}

// print the usage description for the current parser
static void argUsage(argParser* parser) {
    cErrPrintf(TextWhite, "%s ", parser->name);

    charArr shortOpts;
    ARRAY_ALLOC(char, shortOpts, char);

    // gather single character options without an argument
    for(unsigned int i = 0; i < parser->options.capacity; i++) {
        Entry* entry = &parser->options.entries[i];
        if(entry->key.value == NULL) {
            continue;
        }
        optionArg* arg = entry->value;
        if((arg->type == OPT_NONE || arg->type == OPT_ACTION) && arg->hasShortName) {
            PUSH_ARRAY(char, shortOpts, char, arg->shortName);
        }
    }

    // sort the single character options
    qsort(shortOpts.chars, shortOpts.charCount, sizeof(char), charSort);

    // and print them out
    if(shortOpts.charCount > 0) {
        cErrPrintf(TextWhite, "[-%*s] ", shortOpts.charCount, shortOpts.chars);
    }

    // print options without a single character identifier or that take an argument
    for(unsigned int i = 0; i < parser->options.capacity; i++) {
        Entry* entry = &parser->options.entries[i];
        if(entry->key.value == NULL) {
            continue;
        }
        optionArg* arg = entry->value;
        switch(arg->type) {
            case OPT_STRING:
                if(arg->hasShortName) {
                    cErrPrintf(TextWhite, "[-%c", arg->shortName);
                } else {
                    cErrPrintf(TextWhite, "[--%s", arg->longName);
                }
                cErrPrintf(TextWhite, " %s] ", arg->argumentName);
                break;
            default:
                if(!arg->hasShortName) {
                    cErrPrintf(TextWhite, "[--%s]", arg->longName);
                }
                break;
        }
    }

    // prints positional arguments
    for(unsigned int i = 0; i < parser->posArgCount; i++) {
        posArg* arg = &parser->posArgs[i];
        cErrPrintf(TextWhite, "<%s> ", arg->description);
    }

    // end of usage for current parser
    cErrPrintf(TextWhite, "\n");

    argParserArr parsers;
    ARRAY_ALLOC(argParser*, parsers, parser);

    // gather all sub-parsers
    for(unsigned int i = 0; i < parser->modes.capacity; i++) {
        Entry* entry = &parser->modes.entries[i];
        if(entry->key.value == NULL) {
            continue;
        }
        PUSH_ARRAY(argParser*, parsers, parser, entry->value);
    }

    // sort the sub-parsers by name, alphabeticaly
    qsort(parsers.parsers, parsers.parserCount, sizeof(argParser*), parserSort);

    // print usage for all sub-parsers
    for(unsigned int i = 0; i < parsers.parserCount; i++) {
        argUsage(parsers.parsers[i]);
    }
}

// display error caused by incorrect command line
// arguments being passed by the user
static void argError(argParser* parser, const char* message, ...) {
    va_list args;
    va_start(args, message);

    parser->success = false;
    cErrPrintf(TextWhite, "Usage: \n");
    argUsage(parser);
    cErrPrintf(TextWhite, "\nError: ");
    cErrVPrintf(TextRed, message, args);
    cErrPrintf(TextWhite, "\n");

    va_end(args);
}

void argArguments(argParser* parser, int argc, char** argv) {
    // ignore the first argument (program or mode name)
    parser->argc = argc - 1;
    parser->argv = &argv[1];
}

// initialises an argument parser with a given name and the length of that name
static void argInitLen(argParser* parser, const char* name, unsigned int len) {
    initTable(&parser->modes, strHash, strCmp);
    initTable(&parser->options, strHash, strCmp);
    ARRAY_ALLOC(posArg, *parser, posArg);
    parser->success = true;
    parser->parsed = false;
    parser->parseOptions = true;
    parser->usedSubParser = false;
    parser->modeTaken = false;
    parser->currentPosArg = 0;
    parser->name = name;
    parser->nameLength = len;
}

void argInit(argParser* parser, const char* name) {
    argInitLen(parser, name, strlen(name));
}

optionArg* argActionOption(argParser* parser, char shortName, const char* longName, optionAction action, void* ctx) {
    // initialise option normally and then modify it to work with the action
    optionArg* arg = argOption(parser, shortName, longName, false);

    arg->type = OPT_ACTION;
    arg->action = action;
    arg->ctx = ctx;

    return arg;
}

optionArg* argOption(argParser* parser, char shortName, const char* longName, bool takesArg) {

    // error checking for the names
    if(tableHas(&parser->options, (void*)longName)) {
        argInternalError(parser, "Option already exists");
    }

    if(strchr(longName, '=') != NULL) {
        argInternalError(parser, "Long names cannot contain '='");
    }

    if(shortName == '-') {
        argInternalError(parser, "Short names cannot be '-'");
    }

    // allocate a new option
    optionArg* arg = ArenaAlloc(sizeof(optionArg));
    arg->hasShortName = shortName != '\0';
    arg->shortName = shortName;
    arg->longName = longName;
    arg->type = takesArg ? OPT_STRING : OPT_NONE;
    arg->found = false;
    arg->value.as_string = NULL;
    arg->argumentName = "string";

    // add it to the table of options
    tableSet(&parser->options, (void*)longName, arg);
    return arg;
}

argParser* argMode(argParser* parser, const char* name) {
    // error checking
    if(tableHas(&parser->modes, (void*)name)) {
        argInternalError(parser, "Mode already exists");
    }

    argParser* new = ArenaAlloc(sizeof(argParser));

    // the name of a sub-parser is the main-parser's name and the sub-parsers
    // name concatenated

    // +1 for the space
    // +1 for the null byte
    unsigned int nameLen = parser->nameLength + 1 + strlen(name) + 1;
    char* nameArr = ArenaAlloc(sizeof(char) * nameLen);
    strncpy(nameArr, parser->name, parser->nameLength);
    strcat(nameArr, " ");
    strcat(nameArr, name);
    nameArr[nameLen - 1] = '\0';
    argInitLen(new, nameArr, nameLen);

    tableSet(&parser->modes, (void*)name, new);
    return new;
}

void argString(argParser* parser, const char* name) {
    posArg arg;
    arg.description = name;
    arg.type = POS_STRING;
    PUSH_ARRAY(posArg, *parser, posArg, arg);
}

static optionArg* argFindShortName(argParser* parser, char name) {
    // loop through option table to find argument with given short name
    // table is indexed by long name only
    for(unsigned int i = 0; i < parser->options.capacity; i++) {
        Entry* entry = &parser->options.entries[i];
        if(entry->key.value == NULL) {
            continue;
        }
        optionArg* arg = entry->value;
        if(arg->shortName == name) {
            return arg;
        }
    }
    return NULL;
}

// parse argument i as a long (begins with "--") argument
static void argParseLongArg(argParser* parser, int i) {
    optionArg* value;

    // find first equals symbol in the argument
    char* equals = strchr(&parser->argv[i][2], '=');

    // the argument contains an equals symbol, indicating passing an argument to
    // the option this function is parsing, eg --hello=world
    if(equals != NULL) {
        // allocate a new string for the name section of the argument
        char* name = ArenaAlloc(sizeof(char) * (equals - &parser->argv[i][2] + 1));
        strncpy(name, &parser->argv[i][2], equals - &parser->argv[i][2]);
        name[equals - &parser->argv[i][2]] = '\0';

        // lookup the name of the argument
        if(tableGet(&parser->options, name, (void**)&value)) {
            switch(value->type) {
                // only type of option that takes an argument is string
                case OPT_STRING:
                    if(value->found == true) {
                        argError(parser, "Cannot repeat option \"%s\"", name);
                    }
                    // character after the equals symbol
                    value->value.as_string = equals + 1;
                    value->found = true;
                    break;
                default:
                    argError(parser, "Cannot pass argument to option \"%s\" that"
                        " does not require an argument", name);
            }
        } else {
            argError(parser, "Option \"%s\" is undefined", name);
        }
        return;
    }

    // no argument to the option within this argument
    if(tableGet(&parser->options, &parser->argv[i][2], (void**)&value)) {
        switch(value->type) {

            // run the action
            case OPT_ACTION:
                if(!value->action(value->ctx)) {
                    exit(0);
                }
                break;

            // assume argument after the current one is an argument to the current option
            case OPT_STRING:
                i += 1;
                if(i >= parser->argc) {
                    argError(parser, "Option \"%s\" requires an argument", value->longName);
                }
                if(value->found == true) {
                    argError(parser, "Cannot repeat option \"%s\"", value->longName);
                }
                value->value.as_string = parser->argv[i];
                value->found = true;
                break;
            case OPT_NONE:
                if(value->found == true) {
                    argError(parser, "Cannot repeat option \"%s\"", value->longName);
                }
                value->found = true;
        }
    } else {
        argError(parser, "Unknown option \"%s\"", &parser->argv[i][2]);
    }
}

// parse option i assuming that it is a short option (eg "-a")
static void argParseShortArg(argParser* parser, int i, unsigned int argLen) {
    optionArg* first = argFindShortName(parser, parser->argv[i][1]);
    if(first == NULL) {
        argError(parser, "Undefined option \"%c\"", parser->argv[i][1]);
    }

    switch(first->type) {
        // run the action
        case OPT_ACTION:
            if(first->action(first->ctx)) {
                exit(0);
            }
            // TODO: continue parsing compressed single character
            // option list if it is present
            break;
        case OPT_STRING:
            if(argLen > 2) {
                // argument like -llog.txt
                if(first->found == true) {
                    argError(parser, "Cannot repeat option \"%c\"", first->shortName);
                }
                first->value.as_string = &parser->argv[i][2];
                first->found = true;
                break;
            } else {
                // argument like -l "log.txt"
                i += 1;
                if(i >= parser->argc) {
                    argError(parser, "Option \"%c\" requires an argument", first->shortName);
                }
                if(first->found == true) {
                    argError(parser, "Cannot repeat option \"%c\"", first->shortName);
                }
                first->value.as_string = parser->argv[i];
                first->found = true;
                break;
            }
        case OPT_NONE:
            if(first->found == true) {
                argError(parser, "Cannot repeat option \"%c\"", first->shortName);
            }
            first->found = true;

            // argument like "-abc" where none of a, b or c have arguments
            for(unsigned int j = 2; j < argLen; j++) {
                optionArg* arg = argFindShortName(parser, parser->argv[i][j]);
                if(arg == NULL) {
                    argError(parser, "Undefined option \"%c\"", parser->argv[i][j]);
                }
                switch(arg->type) {
                    case OPT_ACTION:
                        if(!arg->action(arg->ctx)) {
                            exit(0);
                        }
                        // TODO: continue parsing compressed single character
                        // option list if it is present
                        break;
                    case OPT_STRING:
                        // do not want to support single character options with arguments
                        // inside compressed argument lists
                        argError(parser, "Option \"%c\" requires an argument so cannot be"
                            " in a multiple option argument",
                            parser->argv[i][j]);
                        break;
                    case OPT_NONE:
                        if(arg->found == true) {
                            argError(parser, "Cannot repeat option \"%c\"", arg->shortName);
                        }
                        arg->found = true;
                }
            }
    }
}

void argParse(argParser* parser) {
    // do not re-run the parser
    if(parser->parsed) {
        return;
    }

    // indicate this parser actually ran, not all modes
    // will be taken.  TODO: is there a difference
    // between mode taken and parsed?
    parser->modeTaken = true;

    // for all command line arguments
    for(int i = 0; i < parser->argc; i++) {

        // is the argument one of the possible modes?
        argParser* new;
        if(tableGet(&parser->modes, parser->argv[i], (void**)&new)) {
            // increment arguments, ignoring the mode name
            argArguments(new, parser->argc, parser->argv);
            argParse(new);

            // if sub-parser failed, propagate faliure
            parser->success &= new->success;

            // flag so errors are not repeated or flagged incorrectly
            parser->usedSubParser = true;
            break;
        }

        size_t argLen = strlen(parser->argv[i]);

        // if '--' parsed, stop parsing option arguments
        if(parser->parseOptions && argLen == 2
            && parser->argv[i][0] == '-' && parser->argv[i][1] == '-') {
            parser->parseOptions = false;
            continue;
        }

        // if other argument beginning with '-' found
        if(parser->parseOptions && parser->argv[i][0] == '-' && argLen > 1) {
            if(parser->argv[i][1] == '-') {
                argParseLongArg(parser, i);
            } else {
                argParseShortArg(parser, i, argLen);
            }
            continue;
        }

        // parse positional argument
        if(parser->currentPosArg < parser->posArgCount) {
            posArg* arg = &parser->posArgs[parser->currentPosArg];
            switch(arg->type) {
                case POS_STRING:
                    arg->value.as_string = parser->argv[i];
            }
            parser->currentPosArg++;
            continue;
        }

        // no successfull use for the argument found
        argError(parser, "unexpected argument \"%s\"", parser->argv[i]);
        return;
    }

    // if not all positional arguments forfilled and using this parser
    if(!parser->usedSubParser && parser->currentPosArg != parser->posArgCount) {
        posArg* arg = &parser->posArgs[parser->currentPosArg];
        argError(parser, "missing required positional argument <%s>", arg->description);
    }
    parser->parsed = true;
}

bool argSuccess(argParser* parser) {
    return parser->success;
}
