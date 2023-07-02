#pragma once
#include "platform/guid.hpp"
#include "misc/log/log_pattern.hpp"

namespace skr {
namespace log {

struct RUNTIME_API LogSink
{
    LogSink(skr_guid_t pattern) SKR_NOEXCEPT;
    virtual ~LogSink() SKR_NOEXCEPT;
    virtual skr_guid_t get_pattern() const SKR_NOEXCEPT { return pattern_; }
    virtual void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT = 0;
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

struct RUNTIME_API LogConsoleSink : public LogSink
{
    LogConsoleSink() SKR_NOEXCEPT;
    virtual ~LogConsoleSink() SKR_NOEXCEPT;
    void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT override;
    void set_style(LogLevel level, EConsoleStyle style) SKR_NOEXCEPT;
    void set_front_color(LogLevel level, EConsoleColor front) SKR_NOEXCEPT;
    void set_back_color(LogLevel level, EConsoleColor back) SKR_NOEXCEPT;

    struct ColorSet
    {
        EConsoleColor f = EConsoleColor::WHILE;
        EConsoleColor b = EConsoleColor::BLACK;
        EConsoleStyle s = EConsoleStyle::NORMAL;
    } color_sets_[static_cast<uint32_t>(LogLevel::kCount)];
    struct BufCache* buf_cache_ = nullptr;
};

struct RUNTIME_API LogFileSink : public LogSink
{
    LogFileSink() SKR_NOEXCEPT;
    virtual ~LogFileSink() SKR_NOEXCEPT;
    void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT override;
    struct CFILE* file_ = nullptr;
};

} // namespace log
} // namespace skr