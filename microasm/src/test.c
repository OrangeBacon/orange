#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "platform.h"
#include "error.h"

void runFile(const char* fileName, Parser* parse, Scanner* scan) {
    const char* file = readFile(fileName);
    const char* fullFileName = resolvePath(fileName);

    ScannerInit(scan, file, fullFileName);
    ParserInit(parse, scan);

    Parse(parse);
}

static bool runTest(const char* path) {
    Scanner scanner;
    Parser parser;

    printf("%s  ->  ", path);
    runFile(path, &parser, &scanner);
    if(parser.ast.errorCount > 0) {
        cErrPrintf(TextRed, "Failed: \n");
    } else {
        cOutPrintf(TextGreen, "Passed\n");
    }
    for(unsigned int i = 0; i < parser.ast.errorCount; i++) {
        printf("  Error[E%04u] at ", parser.ast.errors[i].id);
        TokenPrint(&parser.ast.errors[i].token);
        printf("\n");
    }

    return parser.ast.errorCount == 0;
}

void runTests(const char* directory) {
    printf("Running Tests\n");
    disableErrorPrint();

    if(iterateDirectory(directory, runTest)) {
        cOutPrintf(TextGreen, "All Tests Passed\n");
    } else {
        cErrPrintf(TextRed, "Test Failed\n");
    }

    exit(0);
}