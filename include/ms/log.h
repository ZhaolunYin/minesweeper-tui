#pragma once

#include <stdarg.h>
#include "version.h"

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
} LogLevel;

#define LOG_FILE (PROGRAM_NAME ".log")

#define LOG(...) \
    log_to_file(__FILE__, __LINE__, __func__, __VA_ARGS__)

void log_to_file(const char *file, int line, const char *function, LogLevel level, const char *fmt, ...);
void end_log(void);
