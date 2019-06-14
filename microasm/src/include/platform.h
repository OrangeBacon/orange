#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdarg.h>

// convert an relative path into an absolute path
const char* resolvePath(const char* path);

// terminal text output color
typedef enum TextColor {
#ifdef _WIN32
    TextBlack = 0,
    TextRed = 4,
    TextGreen = 2,
    TextYellow = 6,
    TextBlue = 1,
    TextMagenta = 5,
    TextCyan = 3,
    TextWhite = 7
#else
    TextBlack = 30,
    TextRed,
    TextGreen,
    TextYellow,
    TextBlue,
    TextMagenta,
    TextCyan,
    TextWhite
#endif
} TextColor;

extern bool EnableColor;

// setup color terminal output
void startColor();

// printf a string with a given color to stderr
void cErrPrintf(TextColor color, const char* format, ...);

// printf a forat string and arguments with a given color to stderr
void cErrVPrintf(TextColor color, const char* format, va_list args);

// printf a string with a given color to stdout
void cOutPrintf(TextColor color, const char* format, ...);

// printf a forat string and arguments with a given color to stdout
void cOutVPrintf(TextColor color, const char* format, va_list args);

// get a buffer containing the string contents of the file provided
const char* readFile(const char* fileName);
const char* readFilePtr(FILE* file, const char* fileName);

typedef void(*directoryCallback)(const char* path, const char* file);

void iterateDirectory(const char* basePath, directoryCallback callback);

#endif