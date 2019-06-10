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

void runTests(const char* directory) {
    Scanner scanner;
    Parser parser;

    (void)directory;

    cOutPrintf(TextGreen, "Running Tests\n");
    disableErrorPrint();
    runFile(directory, &parser, &scanner);
    if(parser.ast.errorCount > 0) {
        cErrPrintf(TextRed, "Errors: ");
    }
    for(unsigned int i = 0; i < parser.ast.errorCount; i++) {
        printf("\n  Error %u: code = %u at ", i, parser.ast.errors[i].id);
        TokenPrint(&parser.ast.errors[i].token);
    }
    printf("\nDone");
    exit(0);
}