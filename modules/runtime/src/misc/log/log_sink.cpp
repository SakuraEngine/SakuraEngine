#include "../../pch.hpp"
#include "misc/log.h"
#include "misc/log/log_sink.hpp"
#include "misc/log/log_manager.hpp"
#include "misc/log/logger.hpp"

#include "tracy/Tracy.hpp"

#ifdef _WIN32
#define USE_WIN32_CONSOLE
#endif

namespace skr {
namespace log {

LogSink::LogSink(skr_guid_t pattern) SKR_NOEXCEPT
    : pattern_(pattern)
{

}

LogSink::~LogSink() SKR_NOEXCEPT
{

}

LogConsoleSink::LogConsoleSink() SKR_NOEXCEPT
    : LogSink(LogConstants::kDefaultConsolePatternId)
{
#ifdef USE_WIN32_CONSOLE
    ::SetConsoleOutputCP(CP_UTF8);
#endif
    set_front_color(LogLevel::kTrace, EConsoleColor::WHILE);
    set_front_color(LogLevel::kDebug, EConsoleColor::CYAN);
    set_front_color(LogLevel::kInfo, EConsoleColor::GREEN);

    set_style(LogLevel::kWarning, EConsoleStyle::BOLD);
    set_front_color(LogLevel::kWarning, EConsoleColor::YELLOW);

    set_style(LogLevel::kError, EConsoleStyle::BOLD);
    set_front_color(LogLevel::kError, EConsoleColor::RED);

    // white bold on red background
    set_style(LogLevel::kFatal, EConsoleStyle::BOLD);
    set_back_color(LogLevel::kFatal, EConsoleColor::RED);
    set_front_color(LogLevel::kFatal, EConsoleColor::WHILE);
}

LogConsoleSink::~LogConsoleSink() SKR_NOEXCEPT
{

}

namespace 
{
#ifdef USE_WIN32_CONSOLE
using StyleLiteral = WORD;

template <EConsoleColor> struct FrontColorSpec { [[maybe_unused]] static constexpr WORD value = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; };
template <> struct FrontColorSpec<EConsoleColor::BLACK> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_RED; };
template <> struct FrontColorSpec<EConsoleColor::RED> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_RED; };
template <> struct FrontColorSpec<EConsoleColor::GREEN> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_GREEN; };
template <> struct FrontColorSpec<EConsoleColor::YELLOW> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_RED | FOREGROUND_GREEN; };
template <> struct FrontColorSpec<EConsoleColor::BLUE> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_BLUE; };
template <> struct FrontColorSpec<EConsoleColor::MAGENTA> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_RED | FOREGROUND_BLUE; };
template <> struct FrontColorSpec<EConsoleColor::CYAN> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_GREEN | FOREGROUND_BLUE; };
template <> struct FrontColorSpec<EConsoleColor::WHILE> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; };

template <EConsoleColor> struct BackColorSpec { [[maybe_unused]] static constexpr WORD value = 0; };
template <> struct BackColorSpec<EConsoleColor::BLACK> { [[maybe_unused]] static constexpr WORD value = 0; };
template <> struct BackColorSpec<EConsoleColor::RED> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_RED; };
template <> struct BackColorSpec<EConsoleColor::GREEN> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_GREEN; };
template <> struct BackColorSpec<EConsoleColor::YELLOW> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_RED | BACKGROUND_GREEN; };
template <> struct BackColorSpec<EConsoleColor::BLUE> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_BLUE; };
template <> struct BackColorSpec<EConsoleColor::MAGENTA> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_RED | BACKGROUND_BLUE; };
template <> struct BackColorSpec<EConsoleColor::CYAN> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_GREEN | BACKGROUND_BLUE; };
template <> struct BackColorSpec<EConsoleColor::WHILE> { [[maybe_unused]] static constexpr WORD value = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE; };

template <EConsoleStyle> struct StyleSpec { [[maybe_unused]] static constexpr WORD value = 0; };
template <> struct StyleSpec<EConsoleStyle::BOLD> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_INTENSITY; };
#else
using StyleLiteral = std::string_view;

template <EConsoleColor> struct FrontColorSpec { [[maybe_unused]] static constexpr const char* value = "\033[37m"; };
template <> struct FrontColorSpec<EConsoleColor::BLACK> { [[maybe_unused]] static constexpr const char* value = "\033[30m"; };
template <> struct FrontColorSpec<EConsoleColor::RED> { [[maybe_unused]] static constexpr const char* value = "\033[31m"; };
template <> struct FrontColorSpec<EConsoleColor::GREEN> { [[maybe_unused]] static constexpr const char* value = "\033[32m"; };
template <> struct FrontColorSpec<EConsoleColor::YELLOW> { [[maybe_unused]] static constexpr const char* value = "\033[33m"; };
template <> struct FrontColorSpec<EConsoleColor::BLUE> { [[maybe_unused]] static constexpr const char* value = "\033[34m"; };
template <> struct FrontColorSpec<EConsoleColor::MAGENTA> { [[maybe_unused]] static constexpr const char* value = "\033[35m"; };
template <> struct FrontColorSpec<EConsoleColor::CYAN> { [[maybe_unused]] static constexpr const char* value = "\033[36m"; };
template <> struct FrontColorSpec<EConsoleColor::WHILE> { [[maybe_unused]] static constexpr const char* value = "\033[37m"; };

template <EConsoleColor> struct BackColorSpec { [[maybe_unused]] static constexpr const char* value = "\033[40m"; };
template <> struct BackColorSpec<EConsoleColor::BLACK> { [[maybe_unused]] static constexpr const char* value = "\033[40m"; };
template <> struct BackColorSpec<EConsoleColor::RED> { [[maybe_unused]] static constexpr const char* value = "\033[41m"; };
template <> struct BackColorSpec<EConsoleColor::GREEN> { [[maybe_unused]] static constexpr const char* value = "\033[42m"; };
template <> struct BackColorSpec<EConsoleColor::YELLOW> { [[maybe_unused]] static constexpr const char* value = "\033[43m"; };
template <> struct BackColorSpec<EConsoleColor::BLUE> { [[maybe_unused]] static constexpr const char* value = "\033[44m"; };
template <> struct BackColorSpec<EConsoleColor::MAGENTA> { [[maybe_unused]] static constexpr const char* value = "\033[45m"; };
template <> struct BackColorSpec<EConsoleColor::CYAN> { [[maybe_unused]] static constexpr const char* value = "\033[46m"; };
template <> struct BackColorSpec<EConsoleColor::WHILE> { [[maybe_unused]] static constexpr const char* value = "\033[47m"; };

template <EConsoleStyle> struct StyleSpec { [[maybe_unused]] static constexpr const char* value = "\033[1m"; };
template <> struct StyleSpec<EConsoleStyle::BOLD> { [[maybe_unused]] static constexpr const char* value = ""; };
#endif

static constexpr StyleLiteral GetFrontColor(EConsoleColor front) SKR_NOEXCEPT
{
    switch (front)
    {
        case EConsoleColor::BLACK: return FrontColorSpec<EConsoleColor::BLACK>::value;
        case EConsoleColor::RED: return FrontColorSpec<EConsoleColor::RED>::value;
        case EConsoleColor::GREEN: return FrontColorSpec<EConsoleColor::GREEN>::value;
        case EConsoleColor::YELLOW: return FrontColorSpec<EConsoleColor::YELLOW>::value;
        case EConsoleColor::BLUE: return FrontColorSpec<EConsoleColor::BLUE>::value;
        case EConsoleColor::MAGENTA: return FrontColorSpec<EConsoleColor::MAGENTA>::value;
        case EConsoleColor::CYAN: return FrontColorSpec<EConsoleColor::CYAN>::value;
        case EConsoleColor::WHILE: return FrontColorSpec<EConsoleColor::WHILE>::value;
        default: return FrontColorSpec<EConsoleColor::WHILE>::value;
    }
}


static constexpr StyleLiteral GetBackColor(EConsoleColor front) SKR_NOEXCEPT
{
    switch (front)
    {
        case EConsoleColor::BLACK: return BackColorSpec<EConsoleColor::BLACK>::value;
        case EConsoleColor::RED: return BackColorSpec<EConsoleColor::RED>::value;
        case EConsoleColor::GREEN: return BackColorSpec<EConsoleColor::GREEN>::value;
        case EConsoleColor::YELLOW: return BackColorSpec<EConsoleColor::YELLOW>::value;
        case EConsoleColor::BLUE: return BackColorSpec<EConsoleColor::BLUE>::value;
        case EConsoleColor::MAGENTA: return BackColorSpec<EConsoleColor::MAGENTA>::value;
        case EConsoleColor::CYAN: return BackColorSpec<EConsoleColor::CYAN>::value;
        case EConsoleColor::WHILE: return BackColorSpec<EConsoleColor::WHILE>::value;
        default: return BackColorSpec<EConsoleColor::WHILE>::value;
    }
}

static constexpr StyleLiteral GetStyle(EConsoleStyle style) SKR_NOEXCEPT
{
    switch (style)
    {
        case EConsoleStyle::NORMAL: return StyleSpec<EConsoleStyle::NORMAL>::value;
        case EConsoleStyle::BOLD: return StyleSpec<EConsoleStyle::BOLD>::value;
        default: return 0;
    }
}

#ifdef USE_WIN32_CONSOLE
static const DWORD GetTextAttribute(EConsoleColor front, EConsoleColor back, EConsoleStyle style) SKR_NOEXCEPT
{
    return static_cast<DWORD>(front) | static_cast<DWORD>(back) | static_cast<DWORD>(style);
}
#else
#endif

}

void LogConsoleSink::set_front_color(LogLevel level, EConsoleColor front) SKR_NOEXCEPT
{
    color_sets_[static_cast<uint32_t>(level)].f = front;
}

void LogConsoleSink::set_back_color(LogLevel level, EConsoleColor back) SKR_NOEXCEPT
{
    color_sets_[static_cast<uint32_t>(level)].b = back;
}

void LogConsoleSink::set_style(LogLevel level, EConsoleStyle style) SKR_NOEXCEPT
{
    color_sets_[static_cast<uint32_t>(level)].s = style;
}

void LogConsoleSink::sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT
{
#ifdef USE_WIN32_CONSOLE
    const auto StdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    // get origin
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    ::GetConsoleScreenBufferInfo(StdHandle, &csbiInfo);

    // set color
    const auto L = static_cast<uint32_t>(event.get_level());
    const auto attrs = GetTextAttribute(color_sets_[L].f, color_sets_[L].b, color_sets_[L].s);
    if (csbiInfo.wAttributes != attrs)
        ::SetConsoleTextAttribute(StdHandle, attrs);

    // output to console
    ::WriteConsoleA(StdHandle, content.c_str(),
        static_cast<DWORD>(content.size()), nullptr, nullptr);

    // reset origin
    if (csbiInfo.wAttributes != attrs)
        ::SetConsoleTextAttribute(StdHandle, csbiInfo.wAttributes);
#else

    ::printf("%s\n", content.c_str());

#endif
}

LogFileSink::LogFileSink() SKR_NOEXCEPT
    : LogSink(LogConstants::kDefaultFilePatternId)
{

}

LogFileSink::~LogFileSink() SKR_NOEXCEPT
{

}

void LogFileSink::sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT
{

}

} } // namespace skr::log