#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "platform.h"
#include "error.h"

void runFile(const char* fileName, const char* file, Parser* parse, Scanner* scan) {
    const char* fullFileName = resolvePath(fileName);

    ScannerInit(scan, file, fullFileName);
    ParserInit(parse, scan);

    Parse(parse);
}

static int testCount;
static int passedCount;
static void runTest(const char* path, const char* file) {
    testCount++;
    Scanner scanner;
    Parser parser;

    printf("Testing: %s", path);
    runFile(path, file, &parser, &scanner);
    printf("  ->  ");
    if(parser.ast.errorCount > 0) {
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