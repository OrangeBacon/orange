#include "shared/log.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

static FILE* logFile;

static void logFatalError(const char* message) {
    puts(message);
    fputs(message, logFile);
    logClose();
    exit(0);
}

typedef struct Context {
    const char* file;
    int line;
    const char* function;
    char* string;
    bool written;
} Context;

static Context* contexts;
static unsigned int contextCount = 0;
static unsigned int contextCapacity = 0;
static unsigned int currentContextCount = 0;

static void pushContext(Context* context) {
    if(contextCount + 1 >= contextCapacity) {
        if(contextCapacity < 8) {
            contextCapacity = 8;
        } else {
            contextCapacity *= 2;
        }
        Context* newPtr = malloc(sizeof(Context) * contextCapacity);
        if(newPtr == NULL) {
            logFatalError("Log out of memory while pushing new context\n");
        }
        contexts = newPtr;
    }
    contexts[contextCount] = *context;
    contextCount += 1;
}

static const unsigned int stringMemoryPageSize = 4 * 4096;
static unsigned int stringMemoryCount = 0;
static char** stringMemoryPointers;
static int stringMemoryPointerCount = 0;
static int stringMemoryPointerCapacity = 0;

static char* logStringAlloc(int count) {
    if((stringMemoryPageSize - stringMemoryCount) < (count * sizeof(char))
        || stringMemoryPointerCount == 0) {
        if(stringMemoryPointerCount + 1 >= stringMemoryPointerCapacity) {
            if(stringMemoryPointerCapacity == 0) {
                stringMemoryPointerCapacity = 1;
            } else {
                stringMemoryPointerCapacity *= 2;
            }

            char** newPtr = realloc(stringMemoryPointers, 
                sizeof(char*) * stringMemoryPointerCapacity);
            if(newPtr == NULL) {
                logFatalError("Log out of memory while increasing size "
                    "of string free list\n");
            }
            stringMemoryPointers = newPtr;
        }

        char* newPtr = malloc(sizeof(char) * stringMemoryPageSize);
        if(newPtr == NULL) {
            logFatalError("Log out of memory while allocating new string page\n");
        }
        stringMemoryPointers[stringMemoryPointerCount] = newPtr;
        stringMemoryCount = 0;
        stringMemoryPointerCount += 1;
    }

    char* ret = stringMemoryPointers[stringMemoryPointerCount-1] + stringMemoryCount;
    stringMemoryCount += count * sizeof(char);
    return ret;
}

void logInit() {
    logFile = fopen("log.txt", "w");
    contextCount = 0;
    contextCapacity = 0;
}

int logContext(const char* file, int line, const char* function, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    Context ctx;
    ctx.file = file;
    ctx.line = line;
    ctx.function = function;
    ctx.written = false;

    int count = vsnprintf(NULL, 0, fmt, args)+1;
    ctx.string = logStringAlloc(count);

    va_end(args);
    va_start(args, fmt);

    vsnprintf(ctx.string, count, fmt, args);

    va_end(args);
    pushContext(&ctx);

    return 0;
}

void logContextEnd(int* dummy) {
    (void)dummy;
    contextCount -= 1;
    if(contexts[contextCount].written) {
        currentContextCount -= 1;
    }
}

static int minumumLevel = 0;

void logLog(int level, int line, const char* fmt, ...) {
    if(level < minumumLevel) {
        return;
    }
    va_list args;
    va_start(args, fmt);

    if(contextCount > currentContextCount) {
        for(unsigned int i = currentContextCount; 
            i < (contextCount - currentContextCount); i++) {
            Context* ctx = &contexts[i];
            fprintf(logFile, "%*s%s:%u at %s: %s\n", i*2, "", 
                ctx->file, ctx->line, ctx->function, ctx->string);
            ctx->written = true;
        }
        currentContextCount = contextCount;
    }

    fprintf(logFile, "%*s&:%i ", currentContextCount * 2, "", line);
    if(level >= 1000) {
        fputs("FATAL", logFile);
    } else if(level >= 800) {
        fputs("ERROR", logFile);
    } else if(level >= 600) {
        fputs("WARN", logFile);
    } else if(level >= 400) {
        fputs("INFO", logFile);
    } else if(level >= 200) {
        fputs("DEBUG", logFile);
    } else {
        fputs("TRACE", logFile);
    }
    fprintf(logFile, "(%u) ", level);
    vfprintf(logFile, fmt, args);
    fprintf(logFile, "\n");

    va_end(args);
}

void logClose() {
    fclose(logFile);
    for(int i = 0; i < stringMemoryPointerCount; i++) {
        free(stringMemoryPointers[i]);
    }
    free(stringMemoryPointers);
    free(contexts);
}