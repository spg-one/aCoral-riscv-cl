#include "log.h"

// static const char *level_strings[] = {
//   "LOG_DEBUG", "LOG_TRACE", "LOG_INFO", "LOG_WARN", "LOG_ERROR"
// };

// static const char *level_colors[] = {
//   "\033[0;36m", "\033[0;32m", "\033[0;37m", "\033[0;33m", "\033[0;31m"
// };

// static const char* level_color_end = "\033[0m";

void log_log(int level, const char *file, int line){
    // 创建一个va_list类型的变量，用来存储可变参数列表
    // va_list args;
    // 用va_start宏初始化可变参数列表，第二个参数是可变参数列表的前一个参数
    // va_start(args, fmt);
    // 打印日志的级别，文件名和行号
    printf("[%s%s%s] %s:%d  ", level_colors[level], level_strings[level], level_color_end, file, line);
    // printf(fmt);
    // 用va_arg宏和fmt参数获取可变参数列表中的每一个参数，并用printf函数打印出来
    // vprintf(fmt, args);
    // 用va_end宏结束可变参数的获取
    // va_end(args);
}