#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <dirent.h>
#include "shared/platform.h"
#include "shared/memory.h"

#ifdef _WIN32
#include <windows.h>
#endif

const char* resolvePath(const char* path) {
#ifdef _WIN32
    // _fullpath is defined in microsoft's stdlib.h
    return _fullpath(NULL, path, _MAX_PATH);
#else
    // realpath is defined in linux's stdlib.h
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

// is color printing enabled?
bool EnableColor = false;

void startColor() {
    // assumes it will succeed enabling color
    EnableColor = true;

#ifdef _WIN32
    HandleErr = GetStdHandle(STD_ERROR_HANDLE);
    HandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if(!GetConsoleScreenBufferInfo(HandleErr, &ErrReset) ||
       !GetConsoleScreenBufferInfo(HandleOut, &OutReset)) {
        EnableColor = false;
    }
#endif
    // on linux, nothing is done as color uses vt100
    // escape sequences, rather than standard library calls
}

void cErrPrintf(TextColor color, const char* format, ...) {
    va_list args;
    va_start(args, format);

    cErrVPrintf(color, format, args);

    va_end(args);
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

    va_end(args);
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

void cErrPuts(TextColor color, const char* string) {
#ifdef _WIN32
    if(EnableColor) SetConsoleTextAttribute(HandleOut, color | FOREGROUND_INTENSITY);
    fputs(string, stderr);
    if(EnableColor) SetConsoleTextAttribute(HandleOut, OutReset.wAttributes);
#else
    if(EnableColor) fprintf(stdout, "\x1B[1;%um", color);
    fputs(stdout, string);
    if(EnableColor) fprintf(stdout, "\x1B[0m");
#endif
}

void cOutPuts(TextColor color, const char* string) {
#ifdef _WIN32
    if(EnableColor) SetConsoleTextAttribute(HandleOut, color | FOREGROUND_INTENSITY);
    fputs(string, stdout);
    if(EnableColor) SetConsoleTextAttribute(HandleOut, OutReset.wAttributes);
#else
    if(EnableColor) fprintf(stdout, "\x1B[1;%um", color);
    fputs(stdout, string);
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
    char* buffer = ArenaAlloc((fileSize + 1) * sizeof(char));
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    return buffer;
}

const char pathSeperator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

bool iterateDirectory(const char* basePath, directoryCallback callback) {
    // assumes path max as 1000
    char path[1000];

    // uses dirent.h
    DIR* directory = opendir(basePath);
    struct dirent *entry;

    // path provided was not a directory
    if(!directory) {
        return false;
    }

    bool result = false;

    // foreach entry in the directory
    while((entry = readdir(directory)) != NULL) {
        // ignore current and parent directory
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {

            const char pathSepString[] = {pathSeperator, '\0'};

            // contruct path to found file or folder
            strcpy(path, basePath);
            strcat(path, pathSepString);
            strcat(path, entry->d_name);

            // try to open it as a file. if it fails, it must be a directory
            // so iterate it.
            FILE* file = fopen(path, "r");
            if(file == NULL) {
                fclose(file);
                result = result || iterateDirectory(path, callback);
            } else {
                // otherwise file found, read it and run callback on it.
                callback(path, readFilePtr(file));
                fclose(file);
                result = true;
            }
        }
    }

    return result;
}
