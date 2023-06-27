#pragma once
#include "log_base.hpp"

namespace skr {
namespace log {

struct LogPattern
{
public:
    enum class TimestampPrecision : uint8_t
    {
        None,
        MilliSeconds,
        MicroSeconds,
        NanoSeconds
    };

    enum Attribute : uint32_t
    {
        ascii_time,
        level_id,
        level_name,
        logger_name,
        thread_id,
        thread_name,
        process_id,
        process_name,
        file_name,
        file_line,
        funtion_name,
        message,
        Count
    };
    LogPattern() SKR_NOEXCEPT
    {
        // Set the default pattern
        _set_pattern(
            u8"{ascii_time} [{thread_name}] {file_line} "
            u8"LOG_{level_name} {logger_name} {message}"
        );
    }
    LogPattern(const char8_t* format_pattern, const char8_t* timestamp_format, Timezone timezone)
        // : _timestamp_formatter(timestamp_format, timezone)
    {
        _set_pattern(format_pattern);
    }
    LogPattern(LogPattern const& other) = delete;
    LogPattern(LogPattern&& other) noexcept = delete;
    LogPattern& operator=(LogPattern const& other) = delete;
    LogPattern& operator=(LogPattern&& other) noexcept = delete;
    virtual ~LogPattern() SKR_NOEXCEPT;

protected:
    void _set_pattern(const char8_t* pattern) SKR_NOEXCEPT;
};

} // namespace log
} // namespace skr