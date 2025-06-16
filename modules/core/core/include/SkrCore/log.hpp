#pragma once
#include "SkrCore/log.h"
#include "SkrCore/log/logger.hpp"
#include "SkrCore/memory/sp.hpp"
#include "SkrProfile/profile.h"

#ifdef __cplusplus
    template <typename...Args>
    void skr_log_log_cxx(int level, const char* file, const char* func, const char* line, const char8_t* fmt, Args&&... args)
    {
        SkrZoneScopedN("LogCxx");

        const auto kLogLevel = skr::logging::LogConstants::kLogLevelsLUT[level];
        if (kLogLevel < skr::logging::LogConstants::gLogLevel) return;

        auto logger = skr::logging::Logger::GetDefault();
        const skr::logging::LogSourceData Src = { file, func, line  };
        const auto Event = skr::logging::LogEvent(logger, kLogLevel, Src);

        logger->log(Event, fmt, std::forward<Args>(args)...);
    }

    #define SKR_LOG_FMT_WITH_LEVEL(level, fmt, ...) skr_log_log_cxx((level), __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_BACKTRACE(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_BACKTRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_TRACE(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_TRACE, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_DEBUG(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_DEBUG, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_INFO(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_INFO, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_WARN(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_WARN, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_ERROR(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_ERROR, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)
    #define SKR_LOG_FMT_FATAL(fmt, ...) skr_log_log_cxx(SKR_LOG_LEVEL_FATAL, __FILE__, __LOG_FUNC__, SKR_MAKE_STRING(__LINE__), (fmt), __VA_ARGS__)

namespace skr::logging {

struct SKR_CORE_API LogSink
{
public:
    LogSink(skr_guid_t pattern) SKR_NOEXCEPT;
    virtual ~LogSink() SKR_NOEXCEPT;

    virtual skr_guid_t get_pattern() const SKR_NOEXCEPT { return pattern_; }
    virtual void sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT = 0;
    virtual void flush() SKR_NOEXCEPT {}

protected:
    skr_guid_t pattern_ = LogConstants::kDefaultPatternId;
};

struct SKR_CORE_API LogManager
{
public:
    static LogManager* Get() SKR_NOEXCEPT;

    virtual void InitializeAsyncWorker() SKR_NOEXCEPT = 0;
    virtual void FinalizeAsyncWorker() SKR_NOEXCEPT = 0;
    virtual void FlushAllSinks() SKR_NOEXCEPT = 0;

    virtual skr_guid_t RegisterPattern(const char8_t* pattern) = 0;
    virtual bool RegisterPattern(skr_guid_t guid, const char8_t* pattern) = 0;

    virtual skr_guid_t RegisterSink(skr::UPtr<LogSink> init) = 0;
    virtual bool RegisterSink(skr_guid_t guid, skr::UPtr<LogSink> sink) = 0;
    virtual LogSink* QuerySink(skr_guid_t guid) = 0;
    
protected:
    virtual ~LogManager() = default;
};

} // namespace skr::logging



#endif