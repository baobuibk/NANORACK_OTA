#pragma once
#include <stdarg.h>

typedef void (*bscript_log_fn)(const char* fmt, va_list args);

void BScript_LoggerSetFunc(bscript_log_fn fn);

void BScript_Log(const char* fmt, ...);