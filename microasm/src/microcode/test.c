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
    return runFile(fileName, readFile(fileName), &ast, false);
}

bool runFile(const char* fileName, const char* file, AST* ast, bool testing) {
    const char* fullFileName = resolvePath(fileName);

    Scanner scan;
    Parser parse;
    ScannerInit(&scan, file, fullFileName);
    InitAST(ast);
    ParserInit(&parse, &scan, ast);

    const char* ext = strrchr(fullFileName, '.');
    if(!ext) {
        cErrPrintf(TextRed, "\nCould not detect file type for \"%s\"\n", fullFileName);
        return false;
    } else {
        ext = ext + 1;
    }

    (void)testing;
    if(!strcmp(ext, "uasm")) {
        Parse(&parse);
        VMCoreGen core;
        createEmulator(&core);
        Analyse(&parse, &core);
        return true;
    }

    cErrPrintf(TextRed, "\nUnknown file type \"%s\" when reading file \"%s\"\n", ext, fullFileName);
    return false;
}
