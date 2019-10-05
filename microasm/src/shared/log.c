#include "shared/log.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

FILE* logFile = NULL;

// TODO: Add log tags based on fmt hash
// TODO: Add tag based filtering
// TODO: Add command-line arguments to control logging
// TODO: Prevent trace and debug log calls in release mode

bool logInit() {
    logFile = tmpfile();
    if(logFile == NULL) {
        puts("Error creating temporary file in log initialisation.\n");
        return false;
    }
    return true;
}

void logClose() {
    if(logFile != NULL) {
        fclose(logFile);
    }
}

void fileCopy(FILE* a, FILE* b) {
    char c;
    while((c = fgetc(a)) != EOF) {
        fputc(c, b);
    }
}

bool logSetFile(FILE* file) {
    if(logFile != NULL) {
        fileCopy(logFile, file);
        fclose(logFile);
    }
    if(file == NULL) {
        puts("Specified log file does not exist.\n");
        return false;
    }
    logFile = file;
    return true;
}

LogContext* _log_current_context_ = NULL;
int _log_current_context_depth_ = 0;
static LogContext* logWrittenContext = NULL;
static int logDepth = 0;

void logContextEnd(LogContext* ctx){
    _log_current_context_ = ctx->next;
    if(logWrittenContext == ctx) {
        logWrittenContext = ctx->next;
        logDepth--;
    }
    _log_current_context_depth_ -= 1;
}

static int minumumLevel = 0;

void logLog(int level, int line, const char* fmt, ...) {
    if(level < minumumLevel || fmt[0] == '\0') {
        return;
    }
    va_list args;
    va_start(args, fmt);

    if(logWrittenContext != _log_current_context_) {
        int unprintedCount = _log_current_context_depth_ - logDepth;
        LogContext* ctxs[unprintedCount];
        ctxs[0] = _log_current_context_;
        for(int i = 1; i < unprintedCount; i++) {
            ctxs[i] = ctxs[i-1]->next;
        }
        for(int i = unprintedCount - 1; i >= 0; i--) {
            LogContext* ctx = ctxs[i];
            fprintf(logFile, "%*s%s:%s at %s: \n", logDepth * 2, "", ctx->filename, ctx->line, ctx->function);
            logDepth += 1;
        }
        logWrittenContext = _log_current_context_;
    }

    fprintf(logFile, "%*s&:%i ", logDepth * 2, "", line);
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