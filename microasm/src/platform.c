#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include "platform.h"

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

void startColor() {
    // nothing to do on linux, vt100 terminal commands
    // used instead
#ifdef _WIN32
    HandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HandleErr = GetStdHandle(STD_ERROR_HANDLE);
    if(!GetConsoleScreenBufferInfo(HandleOut, &OutReset) ||
       !GetConsoleScreenBufferInfo(HandleOut, &ErrReset)) {
        WinColorSuccess = false;
    } else {
        WinColorSuccess = true;
    }
#endif
}

void cOutPrintf(TextColor color, const char* format, ...) {
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    if(WinColorSuccess) SetConsoleTextAttribute(HandleOut, color);
    vprintf(format, args);

    // reset to previous color options
    if(WinColorSuccess) SetConsoleTextAttribute(HandleOut, OutReset.wAttributes);
#else
    printf("\x1B[%um", color);  // set forground
    vprintf(format, args);
    printf("\xB[0m");  // reset formatting
#endif
}

void cErrPrintf(TextColor color, const char* format, ...) {
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    if(WinColorSuccess) SetConsoleTextAttribute(HandleOut, color | FOREGROUND_INTENSITY);
    vfprintf(stderr, format, args);
    if(WinColorSuccess) SetConsoleTextAttribute(HandleOut, OutReset.wAttributes);
#else
    fprintf(stderr, "\x1B[1;%um", color);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\x1B[0m");
#endif
}
