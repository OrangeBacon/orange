#ifndef PATH_H
#define PATH_H

#include <stdint.h>
#include <stdio.h>
#include "shared/memory.h"

typedef struct PathStack {
    DEFINE_ARRAY(const char*, path);
} PathStack;

void pathStackInit(PathStack* stack);
void pathStackAddFolderSection(PathStack* stack, const char* path);
FILE* pathStackSearchFile(PathStack* stack, const char* fileName, char** foundName);

const char* pathGetExtension(const char* path);
size_t pathGetFolderLength(const char* path);
FILE* pathStackSearchFileList(const char* filename, const char* requestedExt,
    unsigned int filenameCount, const char** filenames, char** foundFilename);

#endif