#pragma once
#include "platform/configure.h"
#include "containers/string.hpp"

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
    kCount
};

enum class Timezone : uint8_t
{
    LocalTime,
    GmtTime
};

struct RUNTIME_API LogEvent
{
    LogEvent(LogLevel level) SKR_NOEXCEPT;
    
protected:
    friend struct LogPattern;
    bool flush = false;
    LogLevel level;
    uint32_t timestamp;
    uint64_t thread_id;
};

struct LogSourceData
{
    const char* func_;
    const char* file_;
    const char* line_;
};

struct RUNTIME_API LogConstants
{
    static skr::log::LogLevel gLogLevel;
    static const skr_guid_t kDefaultPatternId;
    static constexpr skr::log::LogLevel kLogLevelsLUT[] = {
        skr::log::LogLevel::kTrace, 
        skr::log::LogLevel::kDebug, 
        skr::log::LogLevel::kInfo, 
        skr::log::LogLevel::kWarning, 
        skr::log::LogLevel::kError, 
        skr::log::LogLevel::kFatal 
    };
    static_assert(sizeof(kLogLevelsLUT) / sizeof(kLogLevelsLUT[0]) == (int)skr::log::LogLevel::kCount, "kLogLevelsLUT size mismatch");
};

} // namespace log
} // namespace skr