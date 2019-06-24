
#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <stdarg.h>  /* for va_list */

/** 消息级别定义*/
typedef enum log_level{
    LOG_LEVEL_NONE,     /** 无任何打印 */
    LOG_LEVEL_LOG,      /** 一般运行日志 */
    LOG_LEVEL_ERROR,    /** 错误信息 */
    LOG_LEVEL_WARN,     /** 告警性的消息，背后可能存在错误 */
    LOG_LEVEL_INFO,        /** 提示性的信息输出 */
    LOG_LEVEL_DEBUG,    /** 调试信息输出，用于功能流程诊断 */
}log_level_e;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*print_f)(log_level_e level, const char *file, int line, const char *fmt, va_list ap);
void log_init(print_f printcb);

void log_file(const char *file);
void log_setlevel(log_level_e level);

void log_print(log_level_e level, const char *file, int line, const char *fmt, ...);

/* 实际打印可使用下面的宏 */

#define log_debug(arg...) log_print(LOG_LEVEL_DEBUG, __FILE__, __LINE__, ##arg)

#define log_info(arg...) log_print(LOG_LEVEL_INFO, __FILE__, __LINE__, ##arg)

#define log_warn(arg...) log_print(LOG_LEVEL_WARN, __FILE__, __LINE__, ##arg)

#define log_error(arg...) log_print(LOG_LEVEL_ERROR, __FILE__, __LINE__, ##arg)

#define log_log(arg...) log_print(LOG_LEVEL_LOG, __FILE__, __LINE__, ##arg)

#ifdef __cplusplus
}
#endif

#endif /* !UTIL_LOG_H */
