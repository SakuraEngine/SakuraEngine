/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#pragma once
#include "platform/configure.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_VERSION "0.1.0"

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    struct tm* time;
    void* udata;
    int line;
    int level;
} log_Event;

typedef void (*log_LogFn)(log_Event* ev);
typedef void (*log_LockFn)(bool lock, void* udata);

enum
{
    SKR_LOG_LEVEL_TRACE,
    SKR_LOG_LEVEL_DEBUG,
    SKR_LOG_LEVEL_INFO,
    SKR_LOG_LEVEL_WARN,
    SKR_LOG_LEVEL_ERROR,
    SKR_LOG_LEVEL_FATAL
};

#define SKR_LOG_TRACE(...) log_log(SKR_LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define SKR_LOG_DEBUG(...) log_log(SKR_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define SKR_LOG_INFO(...) log_log(SKR_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define SKR_LOG_WARN(...) log_log(SKR_LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define SKR_LOG_ERROR(...) log_log(SKR_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define SKR_LOG_FATAL(...) log_log(SKR_LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

RUNTIME_API void log_initialize_async_worker();
RUNTIME_API void log_set_level(int level);
RUNTIME_API void log_log(int level, const char* file, int line, const char* fmt, ...);
RUNTIME_API void log_finalize();

#ifdef __cplusplus
}
#endif

