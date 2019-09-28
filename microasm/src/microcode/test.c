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

inline bool runFileName(const char* fileName) {
    AST ast;
    return runFile(fileName, readFile(fileName), &ast, false);
}

bool runFile(const char* fileName, const char* file, AST* ast, bool testing) {
    const char* fullFileName = resolvePath(fileName);

    Scanner scan;
    Parser parse;
    ScannerInit(&scan, file, fullFileName);
    InitAST(ast, fileName);
    ParserInit(&parse, &scan, ast);

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
        expectTestStatements(&parse);
        Parse(&parse);
        VMCoreGen core;
        createEmulator(&core);
        Analyse(&parse, &core);
        return true;
    } else if(isTestFile && !testing) {
        cErrPrintf(TextRed, "\nNot expecting microcode test file while reading \"%s\"\n", ext, fullFileName);
        return false;
    } else
#else
    (void)testing;
#endif
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

#ifdef debug
static int testCount;
static int passedCount;
static void runTest(const char* path, const char* file) {
    testCount++;
    AST ast;

    printf("Testing: %s  ->\n", resolvePath(path));
    bool runSuccess = runFile(path, file, &ast, true);

    unsigned int currentAstError = 0;
    while(currentAstError < ast.errorCount) {
        if(ast.expectedErrorCount >= currentAstError + 1) {
            Error expected = ast.expectedErrors[currentAstError];
            Error actual = ast.errors[currentAstError];
            if(expected.id != actual.id || expected.token.line != actual.token.line || expected.token.column != actual.token.column) {
                runSuccess = false;
                cErrPrintf(TextRed, "  Expected Error[E%04u] at ", ast.expectedErrors[currentAstError].id);
                cErrPrintf(TextRed, "%u:%u\n", ast.expectedErrors[currentAstError].token.line, ast.expectedErrors[currentAstError].token.column);
                cErrPrintf(TextRed, "  Got Error[E%04u] at ", ast.errors[currentAstError].id);
                cErrPrintf(TextRed, "%u:%u\n", ast.errors[currentAstError].token.line, ast.errors[currentAstError].token.column);
            }
        } else {
            runSuccess = false;
            cErrPrintf(TextRed, "  Error[E%04u] at ", ast.errors[currentAstError].id);
            cErrPrintf(TextRed, "%u:%u\n", ast.errors[currentAstError].token.line, ast.errors[currentAstError].token.column);
        }
        currentAstError++;
    }

    if(ast.expectedErrorCount != currentAstError) {
        runSuccess = false;
        for(; currentAstError < ast.expectedErrorCount; currentAstError++) {
            cErrPrintf(TextRed, "  Expected Error[E%04u] at ", ast.expectedErrors[currentAstError].id);
            cErrPrintf(TextRed, "%u:%u\n", ast.expectedErrors[currentAstError].token.line, ast.expectedErrors[currentAstError].token.column);
        }
    }

    if(runSuccess) {
        cOutPrintf(TextGreen, "  Passed\n");
        passedCount++;
    }
}

void runTests(const char* directory) {
    disableErrorPrint();

    if(!iterateDirectory(directory, runTest)) {
        cOutPrintf(TextRed, "Could not find any tests in supplied directory.\n");
        return;
    }

    if(testCount == passedCount) {
        cOutPrintf(TextGreen, "\nAll Tests Passed\n");
        cOutPrintf(TextGreen, "(%i tests executed)\n", passedCount);
    } else {
        cErrPrintf(TextRed, "\nTesting Failed\n");
        cErrPrintf(TextRed, "(%i of %i passed)\n", passedCount, testCount);
    }

    exit(0);
}
#endif
