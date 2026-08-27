#include "ms/log.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "ms.h"

static FILE *log_file = NULL;

static void _init_log_file(void) {
    char *buf = malloc(BUFSIZ);
    if (!buf) {
        printf("Failed to allocate buffer");
        exit(1);
    }
    char *cur = buf;
    char *end = buf + BUFSIZ;
    if (getenv("XDG_STATE_HOME")) {
        cur += snprintf(cur, end - cur, "%s", getenv("XDG_STATE_HOME"));
    }
    else if (getenv("HOME")) {
        cur += snprintf(cur, end - cur, "%s/.local/state", getenv("HOME"));
    }
    cur += snprintf(cur, end - cur, "/%s", PROGRAM_NAME);
    mkdir(buf, 0755);
    cur += snprintf(cur, end - cur, "/%s", LOG_FILE);

    log_file = fopen(buf, "a");
    if (!log_file) {
        printf("Failed to open log file");
        exit(1);
    }
}

static void _print_level(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:
            fprintf(log_file, "[DEBUG] ");
            break;
        case LOG_INFO:
            fprintf(log_file, "[INFO] ");
            break;
        case LOG_WARNING:
            fprintf(log_file, "[WARNING] ");
            break;
        case LOG_ERROR:
            fprintf(log_file, "[ERROR] ");
            break;
    }
}

static void _print_time(void) {
    time_t current_time = time(NULL);
    struct tm *time_info = localtime(&current_time);
    char buf[BUFSIZ];
    strftime(buf, sizeof(buf), "[%d-%m-%Y %H:%M:%S]", time_info);
    fprintf(log_file, "%s ", buf);
}

void log_to_file(const char *file, int line, const char *function, LogLevel level, const char *fmt, ...) {
    if (!log_file)
        _init_log_file();
    _print_level(level);
    _print_time();
    fprintf(log_file, "[%s:%d:%s] ", file, line, function);
    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);
    fprintf(log_file, "\n");
    fflush(log_file);
}

void end_log(void) {
    fclose(log_file);
}
