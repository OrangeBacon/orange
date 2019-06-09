#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "token.h"
#include "parser.h"
#include "ast.h"
#include "error.h"
#include "platform.h"
#include "memory.h"

static void runFile(const char* fileName, Parser* parse, Scanner* scan) {
    const char* file = readFile(fileName);
    const char* fullFileName = resolvePath(fileName);

    ScannerInit(scan, file, fullFileName);
    ParserInit(parse, scan);

    Parse(parse);
}

int main(int argc, char** argv){
    if(argc != 2){
        printf("Usage: microasm <filename>\n");
        return 1;
    }

    startColor();

    ArenaInit();
    Scanner scan;
    Parser parser;

#ifdef debug
    if(strcmp("test", argv[1]) == 0) {
        cOutPrintf(TextGreen, "Running Tests\n");
        disableErrorPrint();
        runFile("../test.uasm", &parser, &scan);
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
#endif

    runFile(argv[1], &parser, &scan);
    PrintMicrocode(&parser.ast);
}