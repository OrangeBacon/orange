#include "shared/platform.h"
#include "shared/memory.h"
#include "shared/arg.h"
#include "microcode/test.h"
#include "emulator/runtime/emu.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser, "microasm");
    argString(&parser, "microcode file");

    argParser* vm = argMode(&parser, "vm");
    argString(vm, "main memory file");
    optionArg* vmVerbose = argOption(vm, 'v', "verbose", false);
    optionArg* vmLogFile = argOption(vm, 'l', "log", true);

#ifdef debug
    argParser* test = argMode(&parser, "test");
    argString(test, "test folder");
#endif
    argArguments(&parser, argc, argv);
    argParse(&parser);

    if(argSuccess(&parser)) {
#ifdef debug
        if(test->modeTaken) {
            runTests(strArg(*test, 0));
        } else 
#endif
        if(vm->modeTaken) {
            runEmulator(strArg(*vm, 0), vmVerbose->found, vmLogFile->value.as_string);
        } else {
            runFileName(strArg(parser, 0));
        }
    }
}
