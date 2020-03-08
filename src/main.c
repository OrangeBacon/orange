#include <string.h>
#include "shared/platform.h"
#include "shared/memory.h"
#include "shared/arg.h"
#include "shared/log.h"
#include "microcode/test.h"
#include "emulator/runtime/emu.h"
#include "emulator/compiletime/runCodegen.h"

int main(int argc, char** argv){
    if(!logInit()) return -1;
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser, "microasm");
    parser.helpMessage = "A series of tools to use with the Orange computer system";
    parser.versionString = "Microasm v0.0.2 alpha\n"
        "Built "__DATE__" "__TIME__"\n";
    parser.printUsage = false;

    optionArg* disableColor = argUniversalOption(&parser, 'c', "no-color", true);
    disableColor->helpMessage = "disable color output";
    optionArg* logFile = argUniversalOptionString(&parser, 'l', "log-file", false);
    logFile->helpMessage = "File name to write logging to.  Default is tempfile voided on program close.";
    optionArg* logLevel = argUniversalOptionInt(&parser, 'd', "debug-level", false);
    logLevel->helpMessage = "Minimum level of importance for log messages to be written. "
        "Fatal is 1000, error is 800, warn is 600, info is 400, debug is 200, trace is 0. "
        "Default value is 0.  Debug and trace logging are disabled when in release mode.";
    optionArg* outputStream = argUniversalOptionString(&parser, 's', "output-stream", false);
    outputStream->helpMessage = "Which stream the output should be written to. "
        "Default value is undefined, \"stdout\" is stdout and \"stderr\" is "
        "stderr.  Useful so errors are not mixed with output from make.  Does "
        "not apply to anything printed from the argument parser.";

    argParser* analyse = argMode(&parser, "analyse");
    analyse->helpMessage = "Parse and analyse a microcode description file";
    posArg* microcode = argString(analyse, "file");
    microcode->helpMessage = "microcode description file to be parsed";

#if BUILD_STAGE > 0
    argParser* vm = argMode(&parser, "vm");
    vm->helpMessage = "Run a microcode binary file in a virtual machine";
    posArg* vmBinary = argString(vm, "file");
    vmBinary->helpMessage = "file containing bytecode to execute in the vm";
    optionArg* vmVerbose = argOption(vm, 'v', "verbose");
    vmVerbose->helpMessage = "enable extra debugging logging";
    optionArg* vmLogFile = argOptionString(vm, 'L', "run-log");
    vmLogFile->argumentName = "path";
    vmLogFile->helpMessage = "vm runtime log file location, default location is stdout";
#endif

#if BUILD_STAGE == 0 || DEBUG_BUILD
    argParser* codegen = argMode(&parser, "codegen");
    codegen->helpMessage = "Generate the source code for the orange virtual machine";
    posArg* codegenInput = argString(codegen, "file");
    codegenInput->helpMessage = "Input microcode file to generate code for";
    posArg* codegenOutput = argString(codegen, "file");
    codegenOutput->helpMessage = "Where to write the generated code";
#endif

#if DEBUG_BUILD
    argParser* printAST = argMode(&parser, "printast");
    posArg* astinput = argString(printAST, "file");
    astinput->helpMessage = "File to print the ast of";
#endif

    argArguments(&parser, argc, argv);
    argParse(&parser);

    if(!argSuccess(&parser)) {
        logClose();
        return -1;
    }

    if(outputStream->found) {
        if(strcmp(outputStream->value.as_string, "stdout") == 0 ) {
            printStreamForceOut();
        } else if(strcmp(outputStream->value.as_string, "stderr") == 0) {
            printStreamForceErr();
        } else {
            cErrPrintf(TextRed, "Unable to recognise selected output stream. "
                "Ignoring, using default value.\n");
        }
    }

    if(logLevel->found) {
        logSetMinLevel(logLevel->value.as_int);
    }

    if(logFile->found) {
        FILE* file = fopen(logFile->value.as_string, "w");
        if(!file) {
            cErrPrintf(TextRed, "Unable to create specified logfile.  Quiting.");
            logClose();
            return 1;
        }
        if(!logSetFile(file)) {
            cErrPrintf(TextRed, "Unable to set logfile to specified file descriptor.  Quitting.");
            logClose();
            return 1;
        }
    }

    if(disableColor->found) {
        EnableColor = false;
    }

#if BUILD_STAGE > 0
    if(vm->parsed) {
        runEmulator(strArg(*vm, 0), vmVerbose->found, vmLogFile->value.as_string);
        logClose();
        return 0;
    }
#endif

#if BUILD_STAGE == 0 || DEBUG_BUILD
    if(codegen->parsed) {
        int result = runCodegen(strArg(*codegen, 0), strArg(*codegen, 1));
        logClose();
        return result;
    }
#endif

#if DEBUG_BUILD
    if(printAST->parsed) {
        runASTPrint(strArg(*printAST, 0));
        logClose();
        return 0;
    }
#endif

    if(analyse->parsed) {
        bool result = !runFileName(strArg(*analyse, 0));
        logClose();
        return result;
    }

    parser.success = false;
    argPrintMessage(&parser);
    logClose();
    return -1;
}
