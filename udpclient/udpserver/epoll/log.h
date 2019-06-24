
#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <stdarg.h>  /* for va_list */

/** ��Ϣ������*/
typedef enum log_level{
    LOG_LEVEL_NONE,     /** ���κδ�ӡ */
    LOG_LEVEL_LOG,      /** һ��������־ */
    LOG_LEVEL_ERROR,    /** ������Ϣ */
    LOG_LEVEL_WARN,     /** �澯�Ե���Ϣ��������ܴ��ڴ��� */
    LOG_LEVEL_INFO,        /** ��ʾ�Ե���Ϣ��� */
    LOG_LEVEL_DEBUG,    /** ������Ϣ��������ڹ���������� */
}log_level_e;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*print_f)(log_level_e level, const char *file, int line, const char *fmt, va_list ap);
void log_init(print_f printcb);

void log_file(const char *file);
void log_setlevel(log_level_e level);

void log_print(log_level_e level, const char *file, int line, const char *fmt, ...);

/* ʵ�ʴ�ӡ��ʹ������ĺ� */

#define log_debug(arg...) log_print(LOG_LEVEL_DEBUG, __FILE__, __LINE__, ##arg)

#define log_info(arg...) log_print(LOG_LEVEL_INFO, __FILE__, __LINE__, ##arg)

#define log_warn(arg...) log_print(LOG_LEVEL_WARN, __FILE__, __LINE__, ##arg)

#define log_error(arg...) log_print(LOG_LEVEL_ERROR, __FILE__, __LINE__, ##arg)

#define log_log(arg...) log_print(LOG_LEVEL_LOG, __FILE__, __LINE__, ##arg)

#ifdef __cplusplus
}
#endif

#endif /* !UTIL_LOG_H */
