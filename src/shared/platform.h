#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

// convert an relative path into an absolute path
const char* resolvePath(const char* path);

// terminal text output color
typedef enum TextColor {
#ifdef _WIN32
    TextBlack = 0,
    TextBlue,
    TextGreen,
    TextCyan,
    TextRed,
    TextMagenta,
    TextYellow,
    TextWhite
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

// is terminal output in color
extern bool EnableColor;

void printStreamForceOut();
void printStreamForceErr();

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

// get a buffer containing the string contents of the filename provided
const char* readFile(const char* fileName);

// get a buffer containing the string contents of the file pointer provided
const char* readFilePtr(FILE* file);

// function called while iterating a directory, passed the path to a file
// and a buffer containing the contents of that file
typedef void(*directoryCallback)(const char* path, const char* file);

// Run a function for every file in a directory, and recurse for
// all folders in the directory
// returns true if it ran the callback
bool iterateDirectory(const char* basePath, directoryCallback callback);

// the character to use to seperate sections in a path
extern const char pathSeperator;

#endif
