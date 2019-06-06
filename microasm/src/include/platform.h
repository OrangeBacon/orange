#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#endif

const char* resolvePath(const char* path);

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
HANDLE HandleOut;
HANDLE HandleErr;
CONSOLE_SCREEN_BUFFER_INFO OutReset;
CONSOLE_SCREEN_BUFFER_INFO ErrReset;
bool WinColorSuccess;
#endif

void startColor();

void cOutPrintf(TextColor color, const char* format, ...);

void cErrPrintf(TextColor color, const char* format, ...);

#endif