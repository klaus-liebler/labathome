#include "stm32g4xx_hal.h"
#include "log.h"


static struct {
  void *udata;
  log_LockFn lock;
  int level;
  bool quiet;
} L;


static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};


void log_set_lock(log_LockFn fn) {
  L.lock = fn;
}


void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(bool enable) {
  L.quiet = enable;
}


void log_log(int level, char const* file, int line, char const* fmt, ...) {

  if (L.quiet || level < L.level) return;
  va_list ap;
  va_start(ap, fmt);
  if (L.lock) { L.lock(true); }
  fprintf(stdout, "%lu %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", HAL_GetTick(), level_colors[level], level_strings[level], file, line);
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\r\n");
  fflush(stdout);
  if (L.lock) { L.lock(false); }
  va_end(ap);
}
