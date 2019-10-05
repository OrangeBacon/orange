#include "shared/platform.h"
#include "shared/memory.h"
#include "shared/arg.h"
#include "shared/log.h"
#include "microcode/test.h"
#include "emulator/runtime/emu.h"

int main(int argc, char** argv){
    if(!logInit()) return -1;
    if(!logSetFile(fopen("log.txt", "w"))) return -1;
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser, "microasm");
    parser.helpMessage = "A series of tools to use with the Orange computer system";
    parser.versionString = "Microasm v0.0.1 alpha testing\n"
        "Built "__DATE__" "__TIME__"\n";

    argParser* analyse = argMode(&parser, "analyse");
    analyse->helpMessage = "Parse and analyse a microcode description file";
    posArg* microcode = argString(analyse, "file");
    microcode->helpMessage = "microcode description file to be parsed";
    optionArg* disableColor = argOption(analyse, 'c', "no-color", false);
    disableColor->helpMessage = "disable color output";

    argParser* vm = argMode(&parser, "vm");
    vm->helpMessage = "Run a microcode binary file in a virtual machine";
    posArg* vmBinary = argString(vm, "file");
    vmBinary->helpMessage = "file containing bytecode to execute in the vm";
    optionArg* vmVerbose = argOption(vm, 'v', "verbose", false);
    vmVerbose->helpMessage = "enable extra debugging logging";
    optionArg* vmLogFile = argOption(vm, 'l', "log", true);
    vmLogFile->argumentName = "path";
    vmLogFile->helpMessage = "log file location, default location is stdout";
    argAddExistingOption(vm, disableColor);

#ifdef debug
    argParser* test = argMode(&parser, "test");
    test->helpMessage = "Run the compiler's internal test suit";
    posArg* testFolder = argString(test, "folder");
    testFolder->helpMessage = "folder to recursivly search for tests in";
    argAddExistingOption(test, disableColor);
#endif

    argArguments(&parser, argc, argv);
    argParse(&parser);

    if(!argSuccess(&parser)) {
        return 0;
    }

    if(disableColor->found) {
        EnableColor = false;
    }

#ifdef debug
    if(test->parsed) {
        runTests(strArg(*test, 0));
    } else 
#endif

    if(vm->parsed) {
        runEmulator(strArg(*vm, 0), vmVerbose->found, vmLogFile->value.as_string);
    } else if(analyse->parsed) {
        runFileName(strArg(*analyse, 0));
    } else {
        parser.success = false;
        argPrintMessage(&parser);
    }
}
