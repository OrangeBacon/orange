#include <string.h>
#include "arg.h"
#include "platform.h"

// display error caused by in-correct arg-parser description
// ony caused by errors in the executable - when raised
// always a bug
static void argInternalError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Implementation Error: %s\n", message);
}

// display error caused by incorrect command line
// arguments being passed by the user
static void argError(argParser* parser, const char* message) {
    parser->success = false;
    cErrPrintf(TextRed, "Error: %s\n", message);
}

void argArguments(argParser* parser, int argc, char** argv) {
    parser->argc = argc - 1;
    parser->argv = &argv[1];
}

void argInit(argParser* parser) {
    initTable(&parser->modes, strHash, strCmp);
    ARRAY_ALLOC(posArg, *parser, posArg);
    parser->success = true;
    parser->parsed = false;
    parser->parseOptions = true;
    parser->usedSubParser = false;
    parser->modeTaken = false;
    parser->currentPosArg = 0;
}

argParser* argMode(argParser* parser, const char* name) {
    if(tableHas(&parser->modes, (void*)name)) {
        argInternalError(parser, "Mode already exists");
    }

    argParser* new = ArenaAlloc(sizeof(argParser));
    argInit(new);
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
        argError(parser, "unexpected argument");
        return;
    }

    // if not all positional arguments forfilled and using this parser
    if(!parser->usedSubParser && parser->currentPosArg != parser->posArgCount) {
        argError(parser, "not enough arguments");
    }
    parser->parsed = true;
}

bool argSuccess(argParser* parser) {
    return parser->success;
}