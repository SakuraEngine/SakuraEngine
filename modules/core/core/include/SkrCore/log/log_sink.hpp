#pragma once
#include "SkrBase/types.h"
#include "SkrCore/log.hpp"
#include "SkrContainersDef/string.hpp"

namespace skr
{
namespace logging
{

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

struct SKR_CORE_API LogConsoleSink : public LogSink {
    LogConsoleSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogConsoleSink() SKR_NOEXCEPT;

    virtual void set_style(LogLevel level, EConsoleStyle style) SKR_NOEXCEPT;
    virtual void set_front_color(LogLevel level, EConsoleColor front) SKR_NOEXCEPT;
    virtual void set_back_color(LogLevel level, EConsoleColor back) SKR_NOEXCEPT;

    void         sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;

protected:
    struct ColorSet {
        EConsoleColor f = EConsoleColor::WHILE;
        EConsoleColor b = EConsoleColor::BLACK;
        EConsoleStyle s = EConsoleStyle::NORMAL;
    } color_sets_[static_cast<uint32_t>(LogLevel::kCount)];
    struct BufCache* buf_cache_ = nullptr;
    uint64_t         bufSize    = 2048;
};

struct SKR_CORE_API LogANSIOutputSink : public LogConsoleSink {
    LogANSIOutputSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogANSIOutputSink() SKR_NOEXCEPT;
};

struct SKR_CORE_API LogConsoleWindowSink : public LogConsoleSink {
    LogConsoleWindowSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogConsoleWindowSink() SKR_NOEXCEPT;
    void         sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;
};

struct SKR_CORE_API LogDebugOutputSink : public LogConsoleSink {
    LogDebugOutputSink(skr_guid_t pattern = LogConstants::kDefaultConsolePatternId) SKR_NOEXCEPT;
    virtual ~LogDebugOutputSink() SKR_NOEXCEPT;
    void         sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT override;
    virtual void flush() SKR_NOEXCEPT override;
};

struct SKR_CORE_API LogFileSink : public LogSink {
    LogFileSink() SKR_NOEXCEPT;
    virtual ~LogFileSink() SKR_NOEXCEPT;
    void          sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT override;
    virtual void  flush() SKR_NOEXCEPT override;
    struct CFILE* file_ = nullptr;
};

} // namespace logging
} // namespace skr