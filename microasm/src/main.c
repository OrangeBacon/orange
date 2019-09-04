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
            runEmulator(strArg(*vm, 0));
        } else {
            runFileName(strArg(parser, 0));
        }
    }
}
