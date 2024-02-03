#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdarg.h>

typedef enum{
    LOG_DEBUG = 0,
    LOG_TRACE,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
}acoral_log_enum;

static const char *level_strings[] = {
  "LOG_DEBUG", "LOG_TRACE", "LOG_INFO", "LOG_WARN", "LOG_ERROR"
};

static const char *level_colors[] = {
  "\033[0;36m", "\033[0;32m", "\033[0;37m", "\033[0;33m", "\033[0;31m"
};

static const char* level_color_end = "\033[0m";

void log_log(int level, const char *file, int line);

// #define ACORAL_LOG_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define ACORAL_LOG_TRACE(format , ...) printf("[\033[0;32mLOG_TRACE\033[0m] %s:%d -> "format, __FILE__, __LINE__ , ##__VA_ARGS__);
// #define ACORAL_LOG_INFO(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
// #define ACORAL_LOG_WARN(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
// #define ACORAL_LOG_ERROR(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)


#endif