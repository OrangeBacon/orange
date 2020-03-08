#include "emulator/compiletime/runCodegen.h"

#include "shared/platform.h"
#include "microcode/scanner.h"
#include "microcode/parser.h"
#include "microcode/analyse.h"
#include "microcode/error.h"
#include "emulator/compiletime/create.h"
#include "emulator/compiletime/template.h"
#include "emulator/compiletime/codegen.h"

int runCodegen(const char* in, const char* out) {
    Scanner scan;
    ScannerInit(&scan, readFile(in), in);

    Parser parse;
    AST ast;
    InitAST(&ast);

    Parse(&parse, &scan, &ast);

    VMCoreGen core;
    createEmulator(&core);
    //Analyse(&parse, &core);
    printErrors(&parse);

    if(parse.hadError) {
        return 1;
    } else {
        coreCodegen(&core, out);
        return 0;
    }
}