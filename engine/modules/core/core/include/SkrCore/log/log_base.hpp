#pragma once
#include "SkrBase/types.h"
#include "SkrBase/config.h"

namespace skr {
namespace logging {

extern const char* kLogMemoryName;
enum class LogLevel : uint32_t
{
    kTrace,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kFatal,
    kBackTrace,
    kCount
};

enum class LogFlushBehavior : uint32_t
{
    kAuto,
    kFlushImmediate,
};

enum class Timezone : uint8_t
{
    LocalTime,
    GmtTime
};

struct LogSourceData
{
    const char* file_;
    const char* func_;
    const char* line_;
};

struct SKR_CORE_API LogEvent
{
    LogEvent(struct Logger* logger, LogLevel level, const LogSourceData& src_data) SKR_NOEXCEPT;
    
    SKR_FORCEINLINE LogLevel get_level() const SKR_NOEXCEPT { return level; }
    SKR_FORCEINLINE uint64_t get_thread_id() const SKR_NOEXCEPT { return thread_id; }

protected:
    friend struct LogPattern;
    bool flush = false;
    LogLevel level;
    int64_t timestamp;
    uint64_t thread_id;
    const char8_t* thread_name;
    struct Logger* logger;
    LogSourceData src_data;
};

struct SKR_CORE_API LogConstants
{
    static skr::logging::LogLevel gLogLevel;
    static skr::logging::LogFlushBehavior gFlushBehavior;
    static constexpr skr::logging::LogFlushBehavior kFlushBehaviorLUT[] = {
        skr::logging::LogFlushBehavior::kAuto,
        skr::logging::LogFlushBehavior::kFlushImmediate
    };
    static constexpr skr::logging::LogLevel kLogLevelsLUT[] = {
        skr::logging::LogLevel::kTrace, 
        skr::logging::LogLevel::kDebug, 
        skr::logging::LogLevel::kInfo, 
        skr::logging::LogLevel::kWarning, 
        skr::logging::LogLevel::kError, 
        skr::logging::LogLevel::kFatal,
        skr::logging::LogLevel::kBackTrace
    };
    static constexpr const char8_t* kLogLevelNameLUT[] = {
        u8"TRACE",
        u8"DEBUG",
        u8"INFO",
        u8"WARN",
        u8"ERROR",
        u8"FATAL",
        u8"BACKTRACE"
    };
    static_assert(sizeof(kLogLevelsLUT) / sizeof(kLogLevelsLUT[0]) == (int)skr::logging::LogLevel::kCount, "kLogLevelsLUT size mismatch");

    static const GUID kDefaultPatternId;
    static const GUID kDefaultConsolePatternId;
    static const GUID kDefaultConsoleSinkId;
    static const GUID kDefaultFilePatternId;
    static const GUID kDefaultFileSinkId;
};

} // namespace logging
} // namespace skr