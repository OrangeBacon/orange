#include "shared/platform.h"
#include "shared/memory.h"
#include "shared/arg.h"
#include "microcode/test.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser);
    argString(&parser, "microcode file");

#ifdef debug
    argParser* test = argMode(&parser, "test");
    argString(test, "test folder");
#endif
    argArguments(&parser, argc, argv);
    argParse(&parser);

    if(argSuccess(&parser)) {
#ifdef debug
        if(test->modeTaken) {
            runTests(test->posArgs[0].value.as_string);
        } else 
#endif
        {
            runFileName(strArg(parser, 0));
        }
    }
}
