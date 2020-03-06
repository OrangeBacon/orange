#include "shared/log.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "shared/platform.h"

FILE* logFile = NULL;

// TODO: Add log tags based on fmt hash
// TODO: Add tag based filtering
// TODO: Add logging to the rest of the program
// TODO: Don't write to file until file provided so messages can be filtered
//       if emitted before minimum log level set.

bool logInit() {
    logFile = tmpfile();
    if(logFile == NULL) {
        puts("Error creating temporary file in log initialisation.\n");
        return false;
    }
    return true;
}

static int errorWarnCount = 0;
void logClose() {
    if(logFile != NULL) {
        fclose(logFile);
    }
#ifdef DEBUG_BUILD
    if(errorWarnCount > 0) {
        cErrPrintf(TextRed, "Encountered %u logged internal errors or "
            "warnings\n", errorWarnCount);
    }
#endif
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

static LogContext firstContext = {
    .filename = "no context"
};
LogContext* _log_current_context_ = &firstContext;
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
void logSetMinLevel(int level) {
    minumumLevel = level;
}

void logLog(int level, int line, const char* file, const char* fmt, ...) {
    if(level < minumumLevel || fmt[0] == '\0') {
        return;
    }

    if(level >= 600) {
        errorWarnCount++;
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

    fprintf(logFile, "%*s", logDepth * 2, "");
    if(strcmp(_log_current_context_->filename, file) == 0) {
        fprintf(logFile, "&");
    } else {
        fprintf(logFile, "%s", file);
    }
    fprintf(logFile, ":%i ", line);
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

    fflush(logFile);

    va_end(args);
}