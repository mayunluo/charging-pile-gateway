#include "log.h"
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

void gateway_log(const char *lvl, const char *fmt, ...) {
    pthread_mutex_lock(&log_lock);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char ts[64];
    snprintf(ts, sizeof(ts), "%04d-%02d-%02d %02d:%02d:%02d",
             tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(stdout, "[%s][%s] ", ts, lvl);
    va_list ap; va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
    fflush(stdout);
    pthread_mutex_unlock(&log_lock);
}
