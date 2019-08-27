#include <stdlib.h>

#include "shared/platform.h"
#include "shared/memory.h"
#include "microcode/scanner.h"
#include "microcode/parser.h"
#include "microcode/analyse.h"
#include "emulator/compiletime/writeEmulator.h"

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
    Microcode* m = Analyse(&parse);
    if(!parse.hadError) {
        writeEmulator(argv[2], m);
    }
}
