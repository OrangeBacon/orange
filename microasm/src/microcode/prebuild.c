#include <stdlib.h>

#include "shared/platform.h"
#include "shared/memory.h"
#include "microcode/test.h"

int main(int argc, char** argv){
    startColor();
    ArenaInit();

    if(argc != 3) {
        cErrPrintf(TextRed, "Argument Error\n");
        exit(-1);
    }

    runFileName(argv[1], argv[2]);
}
