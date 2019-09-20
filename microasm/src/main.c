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
    parser.helpMessage = "A series of tools to use ";

    argParser* analyse = argMode(&parser, "analyse");
    analyse->helpMessage = "Parse and analyse a microcode description file";
    posArg* microcode = argString(analyse, "file");
    microcode->helpMessage = "microcode description file to be parsed";

    argParser* vm = argMode(&parser, "vm");
    vm->helpMessage = "Run a microcode binary file in a virtual machine";
    posArg* vmBinary = argString(vm, "file");
    vmBinary->helpMessage = "file containing bytecode to execute in the vm";
    optionArg* vmVerbose = argOption(vm, 'v', "verbose", false);
    vmVerbose->helpMessage = "enable extra debugging logging";
    optionArg* vmLogFile = argOption(vm, 'l', "log", true);
    vmLogFile->argumentName = "path";
    vmLogFile->helpMessage = "log file location, default location is stdout";

#ifdef debug
    argParser* test = argMode(&parser, "test");
    test->helpMessage = "Run the compiler's internal test suit";
    posArg* testFolder = argString(test, "folder");
    testFolder->helpMessage = "Folder to recursivly search for tests in";
#endif

    argArguments(&parser, argc, argv);
    argParse(&parser);

    if(!argSuccess(&parser)) {
        return 0;
    }

#ifdef debug
    if(test->parsed) {
        runTests(strArg(*test, 0));
    } else 
#endif

    if(vm->parsed) {
        runEmulator(strArg(*vm, 0), vmVerbose->found, vmLogFile->value.as_string);
    } else if(analyse->parsed) {
        runFileName(strArg(parser, 0));
    } else {
        argPrintMessage(&parser);
    }
}
