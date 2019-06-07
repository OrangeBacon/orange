#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "token.h"
#include "parser.h"
#include "ast.h"
#include "platform.h"
#include "memory.h"

static char* readFile(char* fileName);

int main(int argc, char** argv){
    startColor();
    if(argc != 2){
        printf("Usage: microasm <filename>\n");
        return 1;
    }

    ArenaInit();

    char* file = readFile(argv[1]);

    const char* fileName = resolvePath(argv[1]);

    Scanner scan;
    ScannerInit(&scan, file, fileName);

    Parser parse;
    ParserInit(&parse, &scan);
    Parse(&parse);

    PrintMicrocode(&parse.ast);
    free(file);
}

// get a buffer containing the string contents of the file provided
static char* readFile(char* fileName) {
#ifdef debug
    printf("Reading: %s\n", fileName);
#endif

    FILE* file = fopen(fileName, "r");
    if(file == NULL){
        printf("Could not read file \"%s\"\n", fileName);
        exit(1);
    }
    
    // get the length of the file
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // +1 so '\0' can be added
    // buffer should stay allocated for lifetime 
    // of compiler as all tokens reference it
    char* buffer = (char*)ArenaAlloc((fileSize + 1) * sizeof(char));
    if(buffer == NULL){
        printf("Could not enough allocate memory to read file \"%s\".\n", fileName);
        exit(1);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}