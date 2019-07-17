#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "parser.h"
#include "platform.h"
#include "memory.h"
#include "test.h"
#include "arg.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser);
    argString(&parser, "microcode file");

#ifdef Debug
    argParser* test = argMode(&parser, "test");
    argString(test, "test folder");
#endif
    argArguments(&parser, argc, argv);
    argParse(&parser);

    if(argSuccess(&parser)) {
#ifdef Debug
        if(test->modeTaken) {
            runTests(test->posArgs[0].value.as_string);
        } else 
#endif
        {
            Scanner s;
            Parser p;
            runFile(strArg(parser, 0), readFile(strArg(parser, 0)), &p, &s, false);
        }
    }
}
