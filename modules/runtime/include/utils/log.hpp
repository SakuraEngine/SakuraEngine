#pragma once
#include "log.h"

#ifdef __cplusplus
    #include "utils/format.hpp"

    #define SKR_LOG_FMT_TRACE(...) log_log(SKR_LOG_LEVEL_TRACE, __FILE__, __LINE__, "%s", skr::format(__VA_ARGS__).c_str())
    #define SKR_LOG_FMT_DEBUG(...) log_log(SKR_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "%s", skr::format(__VA_ARGS__).c_str())
    #define SKR_LOG_FMT_INFO(...) log_log(SKR_LOG_LEVEL_INFO, __FILE__, __LINE__, "%s", skr::format(__VA_ARGS__).c_str())
    #define SKR_LOG_FMT_WARN(...) log_log(SKR_LOG_LEVEL_WARN, __FILE__, __LINE__, "%s", skr::format(__VA_ARGS__).c_str())
    #define SKR_LOG_FMT_ERROR(...) log_log(SKR_LOG_LEVEL_ERROR, __FILE__, __LINE__, "%s", skr::format(__VA_ARGS__).c_str())
    #define SKR_LOG_FMT_FATAL(...) log_log(SKR_LOG_LEVEL_FATAL, __FILE__, __LINE__, "%s", skr::format(__VA_ARGS__).c_str())
#endif