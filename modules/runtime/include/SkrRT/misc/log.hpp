#pragma once
#include "log.h"
#include "log/logger.hpp"

#include "SkrProfile/profile.h"

#ifdef __cplusplus
    template <typename...Args>
    void skr_log_log_cxx(int level, const char* file, const char* func, const char* line, const char8_t* fmt, Args&&... args)
    {
        SkrZoneScopedN("LogCxx");

        const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];
        if (kLogLevel < skr::log::LogConstants::gLogLevel) return;

        auto logger = skr::log::Logger::GetDefault();
        const skr::log::LogSourceData Src = { file, func, line  };
        const auto Event = skr::log::LogEvent(logger, kLogLevel, Src);

        logger->log(Event, fmt, skr::forward<Args>(args)...);
    }

    #define SKR_LOG_FMT_WITH_LEVEL(level, fmt, ...) skr_log_log_cxx((level), __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_BACKTRACE(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_BACKTRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_TRACE(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_TRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_DEBUG(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_DEBUG, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_INFO(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_INFO, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_WARN(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_WARN, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_ERROR(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_ERROR, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_FATAL(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_FATAL, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
#endif