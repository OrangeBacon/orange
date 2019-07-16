#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "platform.h"
#include "memory.h"
#include "test.h"
#include "arg.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    argParser parser;
    argInit(&parser);
    argParse(&parser, argc, argv);

/*
    if(argc <= 1) {
        cErrPrintf(TextRed, "Not enough arguments provided, "
            "expected microcode file name"
#ifdef debug
            " or \"test\".\n"
#else
            ".\n"
#endif
            );
        exit(0);
    }

#ifdef debug
    if(strcmp("test", argv[1]) == 0) {
        if(argc < 3) {
            cErrPrintf(TextRed, "Not enough arguments provided, "
                "expected name of directory containing compiler tests.\n");
        } else if(argc > 3) {
            cErrPrintf(TextRed, "Too many arguments provided, "
                "expected: microasm test <directory>\n");
        } else {
            runTests(argv[2]);
        }

        // runTests will exit on its own depending on test success,
        // to get here an argument input error will have occured
        exit(1);
    }
#endif

    if(argc != 2) {
        cErrPrintf(TextRed, "Too many arguments provided, "
            "expected: microasm <filename>\n");
        exit(1);
    }

    Scanner scan;
    Parser parser;
    runFile(argv[1], readFile(argv[1]), &parser, &scan, false);
    //PrintMicrocode(&parser.ast);
    */
}
