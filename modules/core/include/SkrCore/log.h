/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#pragma once
#include "SkrBase/config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_VERSION "0.1.0"

enum
{
    SKR_LOG_LEVEL_TRACE,
    SKR_LOG_LEVEL_DEBUG,
    SKR_LOG_LEVEL_INFO,
    SKR_LOG_LEVEL_WARN,
    SKR_LOG_LEVEL_ERROR,
    SKR_LOG_LEVEL_FATAL,
    SKR_LOG_LEVEL_BACKTRACE,
};

enum
{
    SKR_LOG_FLUSH_BEHAVIOR_AUTO,
    SKR_LOG_FLUSH_BEHAVIOR_IMMEDIATE,
};

SKR_EXTERN_C SKR_CORE_API 
void skr_log_set_level(int level);

SKR_EXTERN_C SKR_CORE_API 
void skr_log_set_flush_behavior(int behavior);

SKR_EXTERN_C SKR_CORE_API 
void skr_log_log(int level, const char* file, const char* func, const char* line, const char8_t* u8fmt, ...);

// flush logs of this thread
SKR_EXTERN_C SKR_CORE_API 
void skr_log_flush();

SKR_EXTERN_C SKR_CORE_API 
void skr_log_initialize_async_worker();

SKR_EXTERN_C SKR_CORE_API 
void skr_log_finalize_async_worker();

#define __LOG_FUNC__ __FUNCTION__ 

#define SKR_LOG_WITH_LEVEL(level, ...) skr_log_log((level), __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_BACKTRACE(...) skr_log_log(SKR_LOG_LEVEL_BACKTRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_TRACE(...) skr_log_log(SKR_LOG_LEVEL_TRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_DEBUG(...) skr_log_log(SKR_LOG_LEVEL_DEBUG, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_INFO(...) skr_log_log(SKR_LOG_LEVEL_INFO, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_WARN(...) skr_log_log(SKR_LOG_LEVEL_WARN, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_ERROR(...) skr_log_log(SKR_LOG_LEVEL_ERROR, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)
#define SKR_LOG_FATAL(...) skr_log_log(SKR_LOG_LEVEL_FATAL, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), __VA_ARGS__)

#ifdef __cplusplus
}
#endif

