#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <string.h>

#define __FILENAME__ (__FILE__ + SOURCE_PATH_SIZE)

#define CONTEXT(fmt, ...) \
    int logger_ctx_dummy __attribute__((cleanup(logContextEnd))) \
    = logContext(__FILENAME__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define TRACE(fmt, ...) logLog(0, __LINE__, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) logLog(200, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) logLog(400, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) logLog(600, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) logLog(800, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) logLog(1000, __LINE__, fmt, ##__VA_ARGS__)
#define LOG(level, fmt, ...) logLog(level, __LINE__, fmt, ##__VA_ARGS__)


void logInit();
int logContext(const char* file, int line, const char* function, const char* fmt, ...);
void logContextEnd(int* dummy);
void logLog(int level, int line, const char* fmt, ...);
void logClose();

#endif