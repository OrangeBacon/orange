#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdbool.h>

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define _FILENAME_ ((char*)(__FILE__ + SOURCE_PATH_SIZE))

typedef struct LogContext LogContext;
struct LogContext {
    LogContext* next;
    const char* filename;
    const char* line;
    const char* function;
};

extern LogContext* _log_current_context_;
extern int _log_current_context_depth_;

#define CONTEXT(logger, fmt, ...) \
    LogContext _log_context_ __attribute__((cleanup(logContextEnd))) \
    = {_log_current_context_, __FILE__ + SOURCE_PATH_SIZE, \
        STRINGIFY(__LINE__), __func__}; \
    _log_current_context_depth_ += 1; \
    _log_current_context_ = &_log_context_;\
    logger(fmt, ##__VA_ARGS__)

#define TRACE(fmt, ...) logLog(0, __LINE__, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) logLog(200, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) logLog(400, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) logLog(600, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) logLog(800, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) logLog(1000, __LINE__, fmt, ##__VA_ARGS__)
#define LOG(level, fmt, ...) logLog(level, __LINE__, fmt, ##__VA_ARGS__)


bool logInit();
void logClose();
bool logSetFile(FILE* file);

void logContextEnd(LogContext* ctx);
void logLog(int level, int line, const char* fmt, ...);

#endif