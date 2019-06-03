#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "token.h"

static char* readFile(char* fileName);

int main(int argc, char** argv){
    if(argc != 2){
        printf("Usage: microasm <filename>\n");
        return 1;
    }

    char* file = readFile(argv[1]);

    Scanner scan;
    ScannerInit(&scan, file);

    for (;;) {
        Token token = ScanToken(&scan);
        TokenPrint(&token);
        printf("\n");
        if (token.type == TOKEN_EOF) {
            break;
        }
    }
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
    
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if(buffer == NULL){
        printf("Could not enough allocate memory to read file \"%s\".\n", fileName);
        exit(1);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}