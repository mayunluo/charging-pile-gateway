#ifndef LOG_H
#define LOG_H
#include <stdio.h>
void gateway_log(const char *lvl, const char *fmt, ...);
#define LOGD(fmt, ...) gateway_log("DEBUG", fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) gateway_log("INFO", fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) gateway_log("WARN", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) gateway_log("ERROR", fmt, ##__VA_ARGS__)
#endif
