#pragma once
#include "SkrRT/misc/types.h"
#include "SkrRT/misc/log/log_base.hpp"
#include "SkrRT/containers/string.hpp"

namespace skr {
namespace log {

struct SKR_RUNTIME_API LogSink
{
    LogSink(skr_guid_t pattern) SKR_NOEXCEPT;
    virtual ~LogSink() SKR_NOEXCEPT;
    virtual skr_guid_t get_pattern() const SKR_NOEXCEPT { return pattern_; }
    virtual void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT = 0;
    virtual void flush() SKR_NOEXCEPT {}
protected:
    skr_guid_t pattern_ = LogConstants::kDefaultPatternId;
};   

enum class EConsoleColor : uint16_t
{
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHILE
};

enum class EConsoleStyle : uint16_t
{
    NORMAL,
    HIGHLIGHT
};

struct SKR_RUNTIME_API LogConsoleSink : public LogSink
{
    LogConsoleSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogConsoleSink() SKR_NOEXCEPT;

    virtual void set_style(LogLevel level, EConsoleStyle style) SKR_NOEXCEPT;
    virtual void set_front_color(LogLevel level, EConsoleColor front) SKR_NOEXCEPT;
    virtual void set_back_color(LogLevel level, EConsoleColor back) SKR_NOEXCEPT;

    void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;

protected:
    struct ColorSet
    {
        EConsoleColor f = EConsoleColor::WHILE;
        EConsoleColor b = EConsoleColor::BLACK;
        EConsoleStyle s = EConsoleStyle::NORMAL;
    } color_sets_[static_cast<uint32_t>(LogLevel::kCount)];
    struct BufCache* buf_cache_ = nullptr;
    uint64_t bufSize = 2048;
};

struct SKR_RUNTIME_API LogANSIOutputSink : public LogConsoleSink
{
    LogANSIOutputSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogANSIOutputSink() SKR_NOEXCEPT;
};

struct SKR_RUNTIME_API LogConsoleWindowSink : public LogConsoleSink
{
    LogConsoleWindowSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogConsoleWindowSink() SKR_NOEXCEPT;
    void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;
};

struct SKR_RUNTIME_API LogDebugOutputSink : public LogConsoleSink
{
    LogDebugOutputSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogDebugOutputSink() SKR_NOEXCEPT;
    void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;
};

struct SKR_RUNTIME_API LogFileSink : public LogSink
{
    LogFileSink() SKR_NOEXCEPT;
    virtual ~LogFileSink() SKR_NOEXCEPT;
    void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;
    struct CFILE* file_ = nullptr;
};

} // namespace log
} // namespace skr