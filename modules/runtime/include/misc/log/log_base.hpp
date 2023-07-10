#pragma once
#include "misc/types.h"

namespace skr {
namespace log {

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

struct RUNTIME_API LogEvent
{
    LogEvent(struct Logger* logger, LogLevel level, const LogSourceData& src_data) SKR_NOEXCEPT;
    
    FORCEINLINE LogLevel get_level() const SKR_NOEXCEPT { return level; }
    FORCEINLINE uint64_t get_thread_id() const SKR_NOEXCEPT { return thread_id; }

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

struct RUNTIME_API LogConstants
{
    static skr::log::LogLevel gLogLevel;
    static skr::log::LogFlushBehavior gFlushBehavior;
    static constexpr skr::log::LogFlushBehavior kFlushBehaviorLUT[] = {
        skr::log::LogFlushBehavior::kAuto,
        skr::log::LogFlushBehavior::kFlushImmediate
    };
    static constexpr skr::log::LogLevel kLogLevelsLUT[] = {
        skr::log::LogLevel::kTrace, 
        skr::log::LogLevel::kDebug, 
        skr::log::LogLevel::kInfo, 
        skr::log::LogLevel::kWarning, 
        skr::log::LogLevel::kError, 
        skr::log::LogLevel::kFatal,
        skr::log::LogLevel::kBackTrace
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
    static_assert(sizeof(kLogLevelsLUT) / sizeof(kLogLevelsLUT[0]) == (int)skr::log::LogLevel::kCount, "kLogLevelsLUT size mismatch");

    static const skr_guid_t kDefaultPatternId;
    static const skr_guid_t kDefaultConsolePatternId;
    static const skr_guid_t kDefaultConsoleSinkId;
    static const skr_guid_t kDefaultFilePatternId;
    static const skr_guid_t kDefaultFileSinkId;
};

} // namespace log
} // namespace skr