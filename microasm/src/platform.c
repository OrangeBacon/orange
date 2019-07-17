#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <dirent.h>
#include "platform.h"
#include "memory.h"

#ifdef _WIN32
#include <windows.h>
#endif

const char* resolvePath(const char* path) {
#ifdef _WIN32
    // _fullpath is defined in microsoft's stdlib.h
    return _fullpath(NULL, path, _MAX_PATH);
#else
    char* buf = malloc(PATH_MAX + 1);
    char* ret = realpath(path, buf);
    if(ret) {
        return buf;
    } else {
        return path;
    }
#endif
}

#ifdef _WIN32
// stderr handle
static HANDLE HandleErr;

// inital formatting of stderr
static CONSOLE_SCREEN_BUFFER_INFO ErrReset;

// stdout handle
static HANDLE HandleOut;

// inital formatting of stdout
static CONSOLE_SCREEN_BUFFER_INFO OutReset;
#endif

bool EnableColor = false;

void startColor() {
    EnableColor = true;
#ifdef _WIN32
    HandleErr = GetStdHandle(STD_ERROR_HANDLE);
    HandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GetConsoleScreenBufferInfo(HandleErr, &ErrReset) || 
       !GetConsoleScreenBufferInfo(HandleOut, &OutReset)) {
        EnableColor = false;
    }
#endif
}

void cErrPrintf(TextColor color, const char* format, ...) {
    va_list args;
    va_start(args, format);

    cErrVPrintf(color, format, args);
}

void cErrVPrintf(TextColor color, const char* format, va_list args) {
#ifdef _WIN32
    if(EnableColor) SetConsoleTextAttribute(HandleErr, color | FOREGROUND_INTENSITY);
    vfprintf(stderr, format, args);
    if(EnableColor) SetConsoleTextAttribute(HandleErr, ErrReset.wAttributes);
#else
    if(EnableColor) fprintf(stderr, "\x1B[1;%um", color);
    vfprintf(stderr, format, args);
    if(EnableColor) fprintf(stderr, "\x1B[0m");
#endif
}

void cOutPrintf(TextColor color, const char* format, ...) {
    va_list args;
    va_start(args, format);

    cOutVPrintf(color, format, args);
}

void cOutVPrintf(TextColor color, const char* format, va_list args) {
#ifdef _WIN32
    if(EnableColor) SetConsoleTextAttribute(HandleOut, color | FOREGROUND_INTENSITY);
    vfprintf(stdout, format, args);
    if(EnableColor) SetConsoleTextAttribute(HandleOut, OutReset.wAttributes);
#else
    if(EnableColor) fprintf(stdout, "\x1B[1;%um", color);
    vfprintf(stdout, format, args);
    if(EnableColor) fprintf(stdout, "\x1B[0m");
#endif
}

// get a buffer containing the string contents of the file provided
const char* readFile(const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if(file == NULL){
        printf("Could not read file \"%s\"\n", fileName);
        exit(1);
    }
    return readFilePtr(file);
}

const char* readFilePtr(FILE* file) {
    // get the length of the file
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // +1 so '\0' can be added
    // buffer should stay allocated for lifetime 
    // of compiler as all tokens reference it
    char* buffer = ArenaAlloc((fileSize + 1) * sizeof(char));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}

static const char* pathSeperator =
#ifdef _WIN32
    "\\";
#else
    "/";
#endif

void iterateDirectory(const char* basePath, directoryCallback callback) {
    char path[1000];
    DIR* directory = opendir(basePath);
    struct dirent *entry;

    if(!directory) {
        return;
    }

    while((entry = readdir(directory)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strcpy(path, basePath);
            strcat(path, pathSeperator);
            strcat(path, entry->d_name);
            
            FILE* file = fopen(path, "r");
            if(file == NULL) {
                fclose(file);
                iterateDirectory(path, callback);
            } else {
                callback(path, readFilePtr(file));
                fclose(file);
            }
        }
    }
}