#include "shared/path.h"

#include <string.h>
#include "shared/platform.h"

void pathStackInit(PathStack* stack) {
    ARRAY_ALLOC(const char*, *stack, path);
}

void pathStackAddFolderSection(PathStack* stack, const char* path) {
    size_t len = pathGetFolderLength(path);
    
    char* buf = ArenaAlloc(sizeof(char) * len + 1);
    strncpy(buf, path, len);
    buf[len] = '\0';
    PUSH_ARRAY(const char*, *stack, path, resolvePath(buf));
}

FILE* pathStackSearchFile(PathStack* stack, const char* fileName, char** foundName) {
    size_t fileNameLen = strlen(fileName);
    for(unsigned int i = 0; i < stack->pathCount; i++) {
        size_t stackPathLen = strlen(stack->paths[i]);
        char* buf = ArenaAlloc(sizeof(char) * (fileNameLen + stackPathLen + 2));

        const char pathSepStr[2] = {pathSeperator, '\0'};
        strcpy(buf, stack->paths[i]);
        strcat(buf, pathSepStr);
        strcat(buf, fileName);
        FILE* file = fopen(buf, "r");
        if(file != NULL) {
            *foundName = buf;
            return file;
        }
    }

    return NULL;
}

const char* pathGetExtension(const char* path) {
    char* lastDot = strrchr(path, '.');
    if(lastDot == NULL) {
        return NULL;
    }

    char* lastSlash = strrchr(path, pathSeperator);
    if(lastSlash != NULL && lastDot < lastSlash) {
        return NULL;
    }

    if(*(lastDot + 1) == '\0') {
        return NULL;
    }

    return lastDot + 1;
}

size_t pathGetFolderLength(const char* path) {
    char* lastSlash = strrchr(path, pathSeperator);
    if(lastSlash == NULL) {
        return 0;
    }

    return lastSlash - path;
}

FILE* pathStackSearchFileList(const char* filename, const char* requestedExt,
    unsigned int filenameCount, const char** filenames, char** foundFilename) {
    const char* ext = pathGetExtension(filename);
    if(ext == NULL || strcmp(ext, filename) != 0) {
        char* tempBuffer = ArenaAlloc(sizeof(char) * (strlen(filename) + strlen(requestedExt) + 2));
        strcpy(tempBuffer, filename);
        strcat(tempBuffer, ".");
        strcat(tempBuffer, requestedExt);
        filename = tempBuffer;
    }

    PathStack searchList;
    pathStackInit(&searchList);

    pathStackAddFolderSection(&searchList, ".");
    for(unsigned int i = 0; i < filenameCount; i++) {
        pathStackAddFolderSection(&searchList, filenames[i]);
    }

    return pathStackSearchFile(&searchList, filename, foundFilename);
}