#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>


//to avoid intellisense errors, see https://github.com/microsoft/vscode-cpptools/issues/11164
#if __INTELLISENSE__
#define __FILE_NAME__  __FILE__
#endif

#ifdef __cplusplus
 extern "C" {
#endif


typedef void (*log_LockFn)(bool lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE_NAME__, __LINE__, __VA_ARGS__)

void log_log(int level, char const* file, int line, char const* fmt, ...);
void log_set_lock(log_LockFn);
void log_set_level(int level);
void log_set_quiet(bool enable);

#ifdef __cplusplus
}
#endif

