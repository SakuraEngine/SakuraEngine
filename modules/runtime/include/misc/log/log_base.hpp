#pragma once
#include "platform/configure.h"

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
    bool flush = false;
    LogLevel level;
    uint64_t thread_id;
    uint64_t timestamp;
};

} // namespace log
} // namespace skr