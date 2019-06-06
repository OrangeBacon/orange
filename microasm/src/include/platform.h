#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#endif

// convert an relative path into an absolute path
const char* resolvePath(const char* path);

// terminal text output color
typedef enum TextColor {
#ifdef _WIN32
    TextBlack = 0,
    TextRed = FOREGROUND_RED,
    TextGreen = FOREGROUND_GREEN,
    TextYellow = FOREGROUND_GREEN | FOREGROUND_RED,
    TextBlue = FOREGROUND_BLUE,
    TextMagenta = FOREGROUND_BLUE | FOREGROUND_RED,
    TextCyan = FOREGROUND_BLUE | FOREGROUND_GREEN,
    TextWhite = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
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

#ifdef _WIN32
// stdout handle
HANDLE HandleOut;

// stderr handle
HANDLE HandleErr;

// initial formatting of stdout
CONSOLE_SCREEN_BUFFER_INFO OutReset;

// inital formatting of stderr
CONSOLE_SCREEN_BUFFER_INFO ErrReset;

// have the handles been aquired successfuly?
bool WinColorSuccess;
#endif

// setup color terminal output
void startColor();

// print a string with a given color to stdout
void cOutPrintf(TextColor color, const char* format, ...);

// print a string with a given color to stderr
void cErrPrintf(TextColor color, const char* format, ...);

#endif