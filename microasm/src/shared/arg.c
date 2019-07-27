#include <string.h>
#include "shared/arg.h"
#include "shared/platform.h"

// display error caused by in-correct arg-parser description
// ony caused by errors in the executable - when raised
// always a bug
static void argInternalError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Implementation Error: %s\n", message);
}

static void argUsage(argParser* parser) {
    cErrPrintf(TextWhite, "%s ", parser->name);
    for(unsigned int i = 0; i < parser->posArgCount; i++) {
        posArg* arg = &parser->posArgs[i];
        cErrPrintf(TextWhite, "<%s> ", arg->description);
    }

    cErrPrintf(TextWhite, "\n");

    for(unsigned int i = 0; i < parser->modes.capacity; i++) {
        Entry* entry = &parser->modes.entries[i];
        if(entry->key.value == NULL) {
            continue;
        }
        argUsage(entry->value);
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
}

void argArguments(argParser* parser, int argc, char** argv) {
    parser->argc = argc - 1;
    parser->argv = &argv[1];
}

static void argInitLen(argParser* parser, const char* name, unsigned int len) {
    initTable(&parser->modes, strHash, strCmp);
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

argParser* argMode(argParser* parser, const char* name) {
    if(tableHas(&parser->modes, (void*)name)) {
        argInternalError(parser, "Mode already exists");
    }

    argParser* new = ArenaAlloc(sizeof(argParser));

    unsigned int nameLen = parser->nameLength + 1 + strlen(name);
    char* nameArr = ArenaAlloc(sizeof(char) * nameLen);
    strncpy(nameArr, parser->name, parser->nameLength);
    strcat(nameArr, " ");
    strcat(nameArr, name);
    argInitLen(new, nameArr, nameLen);

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
            // propagate options
            new->parseOptions = parser->parseOptions;
            // increment arguments, ignoring the mode name
            argArguments(new, parser->argc, parser->argv);
            argParse(new);

            // if sub-parser failed, propagate faliure
            parser->success &= new->success;
            // flag so errors are not repeated or flagged incorrectly
            parser->usedSubParser = true;
            break;
        }

        // if '--' parsed, stop parsing option arguments
        if(parser->parseOptions && strlen(parser->argv[i]) == 2 
            && parser->argv[i][0] == '-' && parser->argv[i][1] == '-') {
            parser->parseOptions = false;
            continue;
        }

        // parse positional argument
        if(parser->currentPosArg < parser->posArgCount) {
            posArg* arg = &parser->posArgs[parser->currentPosArg];
            switch(arg->type) {
                case TYPE_STRING:
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