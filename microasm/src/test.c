#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test.h"
#include "platform.h"
#include "error.h"

bool runFile(const char* fileName, const char* file, Parser* parse, Scanner* scan, bool testing) {
    const char* fullFileName = resolvePath(fileName);

    ScannerInit(scan, file, fullFileName);
    ParserInit(parse, scan);

    const char* ext = strrchr(fullFileName, '.');
    if(!ext) {
        cErrPrintf(TextRed, "\nCould not detect file type for \"%s\"\n", fullFileName);
        return false;
    } else {
        ext = ext + 1;
    }

#ifdef debug
    int isTestFile = !strcmp(ext, "uasmt");
    if(isTestFile && testing) {
        expectTestStatements(parse);
        Parse(parse);
        return true;
    } else if(isTestFile && !testing) {
        cErrPrintf(TextRed, "\nNot expecting microcode test file while reading \"%s\"\n", ext, fullFileName);
        return false;
    } else
#else
    (void)testing;
#endif
    if(!strcmp(ext, "uasm")) {
        Parse(parse);
        return true;
    }

    cErrPrintf(TextRed, "\nUnknown file type \"%s\" when reading file \"%s\"\n", ext, fullFileName);
    return false;
}

static int testCount;
static int passedCount;
static void runTest(const char* path, const char* file) {
    testCount++;
    Scanner scanner;
    Parser parser;

    printf("Testing: %s", path);
    bool runSuccess = runFile(path, file, &parser, &scanner, true);
    printf("  ->  ");
    if(parser.ast.errorCount > 0 || !runSuccess) {
        cErrPrintf(TextRed, "Failed: \n");
    } else {
        cOutPrintf(TextGreen, "Passed\n");
        passedCount++;
    }
    for(unsigned int i = 0; i < parser.ast.errorCount; i++) {
        printf("  Error[E%04u] at ", parser.ast.errors[i].id);
        TokenPrint(&parser.ast.errors[i].token);
        printf("\n");
    }
    for(unsigned int i = 0; i < parser.ast.expectedErrorCount; i++) {
        printf("  ExpectedError[E%04u] at ", parser.ast.expectedErrors[i].id);
        printf("%u:%u", parser.ast.expectedErrors[i].token.line, parser.ast.expectedErrors[i].token.column);
        printf("\n");
    }
}

void runTests(const char* directory) {
    printf("Running Tests\n");
    disableErrorPrint();

    iterateDirectory(directory, runTest);

    if(testCount == passedCount) {
        cOutPrintf(TextGreen, "All Tests Passed\n");
        cOutPrintf(TextGreen, "(%i tests executed)\n", passedCount);
    } else {
        cErrPrintf(TextRed, "Testing Failed\n");
        cErrPrintf(TextRed, "(%i of %i passed)\n", passedCount, testCount);
    }

    exit(0);
}