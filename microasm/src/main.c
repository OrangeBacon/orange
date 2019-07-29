#include "shared/platform.h"
#include "shared/memory.h"
#include "shared/arg.h"
#include "microcode/test.h"
#include "emulator/emu.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser, "microasm");
    argString(&parser, "microcode file");
    optionArg* output = argOption(&parser, 'o', "output", true);
    output->value.as_string = "\0";

    argParser* vm = argMode(&parser, "vm");

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
        if(vm->modeTaken) {
            emulator();
        } else {
            runFileName(strArg(parser, 0), output->value.as_string);
        }
    }
}
