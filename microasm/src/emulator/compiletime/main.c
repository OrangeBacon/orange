#include <stdlib.h>

#include "shared/platform.h"
#include "shared/memory.h"
#include "microcode/scanner.h"
#include "microcode/parser.h"
#include "microcode/analyse.h"
#include "emulator/compiletime/template.h"
#include "emulator/compiletime/codegen.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    if(argc != 3) {
        cErrPrintf(TextRed, "Argument Error\n");
        exit(-1);
    }

    Scanner scan;
    ScannerInit(&scan, readFile(argv[1]), argv[1]);

    Parser parse;
    ParserInit(&parse, &scan);

    Parse(&parse);

    VMCoreGen core;
    createEmulator(&core);
    Analyse(&parse, &core);

    if(!parse.hadError) {
        coreCodegen(&core, argv[2]);
    }
}
