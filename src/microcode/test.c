#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shared/platform.h"
#include "emulator/compiletime/template.h"
#include "microcode/test.h"
#include "microcode/error.h"
#include "microcode/analyse.h"
#include "microcode/scanner.h"
#include "microcode/parser.h"

// TODO: re-implement microcode parser test suite
// TODO: re-write microcode tests

inline bool runFileName(const char* fileName) {
    AST ast;
    return runFile(fileName, readFile(fileName), &ast);
}

bool runFile(const char* fileName, const char* file, AST* ast) {
    const char* fullFileName = resolvePath(fileName);

    Scanner scan;
    Parser parse;
    ScannerInit(&scan, file, fullFileName);
    InitAST(ast);

    const char* ext = strrchr(fullFileName, '.');
    if(!ext) {
        cErrPrintf(TextRed, "\nCould not detect file type for \"%s\"\n", fullFileName);
        return false;
    } else {
        ext = ext + 1;
    }

    if(!strcmp(ext, "uasm")) {
        Parse(&parse, &scan, ast);
        VMCoreGen core;
        createEmulator(&core);
        //Analyse(&parse, &core);
        printErrors(&parse);
        return true;
    }

    cErrPrintf(TextRed, "\nUnknown file type \"%s\" when reading file \"%s\"\n", ext, fullFileName);
    return false;
}

void runASTPrint(const char* fileName) {
    const char* fullFileName = resolvePath(fileName);

    AST ast;
    Scanner scan;
    Parser parse;
    ScannerInit(&scan, readFile(fileName), fullFileName);
    InitAST(&ast);
    Parse(&parse, &scan, &ast);
    printErrors(&parse);
    PrintAST(&ast);
}
