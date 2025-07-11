#include "SkrCore/process.h"
#include "SkrCore/log/log_sink.hpp"
#include "log_manager.hpp"

#include <stdio.h> // FILE
#include "SkrProfile/profile.h"

#ifdef _WIN32
#define USE_WIN32_CONSOLE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <io.h>
#endif

namespace skr {
namespace logging {

struct BufCache
{
    std::string buf;
};

LogSink::LogSink(skr_guid_t pattern) SKR_NOEXCEPT
    : pattern_(pattern)
{

}

LogSink::~LogSink() SKR_NOEXCEPT
{
    flush();
}

LogConsoleSink::LogConsoleSink(skr_guid_t pattern) SKR_NOEXCEPT
    : LogSink(pattern), buf_cache_(SkrNew<BufCache>())
{
    set_front_color(LogLevel::kTrace, EConsoleColor::WHILE);
    set_front_color(LogLevel::kDebug, EConsoleColor::CYAN);
    set_front_color(LogLevel::kInfo, EConsoleColor::GREEN);

    set_style(LogLevel::kWarning, EConsoleStyle::HIGHLIGHT);
    set_front_color(LogLevel::kWarning, EConsoleColor::YELLOW);

    set_style(LogLevel::kError, EConsoleStyle::HIGHLIGHT);
    set_front_color(LogLevel::kError, EConsoleColor::RED);

    // white hignlight on red background
    set_style(LogLevel::kFatal, EConsoleStyle::HIGHLIGHT);
    set_back_color(LogLevel::kFatal, EConsoleColor::RED);
    set_front_color(LogLevel::kFatal, EConsoleColor::WHILE);

    set_style(LogLevel::kBackTrace, EConsoleStyle::HIGHLIGHT);
    set_front_color(LogLevel::kBackTrace, EConsoleColor::MAGENTA);
}

LogConsoleSink::~LogConsoleSink() SKR_NOEXCEPT
{
    if (buf_cache_)
        SkrDelete(buf_cache_);
}

LogANSIOutputSink::LogANSIOutputSink(skr_guid_t pattern) SKR_NOEXCEPT
    : LogConsoleSink(pattern)
{
    ::setvbuf(stdout, NULL, _IOFBF, bufSize);
#if SKR_PLAT_WINDOWS
    ::SetConsoleOutputCP(CP_UTF8);
#endif
}

LogANSIOutputSink::~LogANSIOutputSink() SKR_NOEXCEPT
{

}

LogConsoleWindowSink::LogConsoleWindowSink(skr_guid_t pattern) SKR_NOEXCEPT
    : LogConsoleSink(pattern)
{
#ifdef USE_WIN32_CONSOLE
    const auto minLength = bufSize;
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) 
    {
        ::FreeConsole();
        if (auto ret = ::AllocConsole())
        {
            CONSOLE_SCREEN_BUFFER_INFO conInfo;
            ::GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
            if (conInfo.dwSize.Y < minLength)
                conInfo.dwSize.Y = (SHORT)minLength;
            ::SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);

            freopen("CONIN$", "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
        }
    }
    const auto StdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::SetConsoleMode(StdHandle, 
        ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_LVB_GRID_WORLDWIDE |
        ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT
    );
    ::SetConsoleOutputCP(CP_UTF8);
#endif
}

LogConsoleWindowSink::~LogConsoleWindowSink() SKR_NOEXCEPT
{

}

LogDebugOutputSink::LogDebugOutputSink(skr_guid_t pattern) SKR_NOEXCEPT
    : LogConsoleSink(pattern)
{

}

LogDebugOutputSink::~LogDebugOutputSink() SKR_NOEXCEPT
{

}

namespace 
{
#ifdef USE_WIN32_CONSOLE
struct Win
{
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
template <> struct StyleSpec<EConsoleStyle::HIGHLIGHT> { [[maybe_unused]] static constexpr WORD value = FOREGROUND_INTENSITY; };
};
#endif

struct ANSI
{
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

template <EConsoleColor> struct BackColorSpec { [[maybe_unused]] static constexpr const char* value = ""; };
template <> struct BackColorSpec<EConsoleColor::BLACK> { [[maybe_unused]] static constexpr const char* value = ""; };
template <> struct BackColorSpec<EConsoleColor::RED> { [[maybe_unused]] static constexpr const char* value = "\033[41m"; };
template <> struct BackColorSpec<EConsoleColor::GREEN> { [[maybe_unused]] static constexpr const char* value = "\033[42m"; };
template <> struct BackColorSpec<EConsoleColor::YELLOW> { [[maybe_unused]] static constexpr const char* value = "\033[43m"; };
template <> struct BackColorSpec<EConsoleColor::BLUE> { [[maybe_unused]] static constexpr const char* value = "\033[44m"; };
template <> struct BackColorSpec<EConsoleColor::MAGENTA> { [[maybe_unused]] static constexpr const char* value = "\033[45m"; };
template <> struct BackColorSpec<EConsoleColor::CYAN> { [[maybe_unused]] static constexpr const char* value = "\033[46m"; };
template <> struct BackColorSpec<EConsoleColor::WHILE> { [[maybe_unused]] static constexpr const char* value = "\033[47m"; };

template <EConsoleStyle> struct StyleSpec { [[maybe_unused]] static constexpr const char* value = ""; };
template <> struct StyleSpec<EConsoleStyle::HIGHLIGHT> { [[maybe_unused]] static constexpr const char* value = "\033[1m"; };
};

template<template<EConsoleColor C> class FrontColorSpec>
static constexpr auto GetFrontColor(EConsoleColor front) SKR_NOEXCEPT
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

template<template<EConsoleColor C> class BackColorSpec>
static constexpr auto GetBackColor(EConsoleColor front) SKR_NOEXCEPT
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

template<template<EConsoleStyle S> class StyleSpec>
static constexpr auto GetStyle(EConsoleStyle style) SKR_NOEXCEPT
{
    switch (style)
    {
        case EConsoleStyle::NORMAL: return StyleSpec<EConsoleStyle::NORMAL>::value;
        case EConsoleStyle::HIGHLIGHT: return StyleSpec<EConsoleStyle::HIGHLIGHT>::value;
        default: return StyleSpec<EConsoleStyle::NORMAL>::value;
    }
}

#ifdef USE_WIN32_CONSOLE
static const DWORD GetTextAttribute(EConsoleColor front, EConsoleColor back, EConsoleStyle style) SKR_NOEXCEPT
{
    return GetFrontColor<Win::FrontColorSpec>(front) | GetBackColor<Win::BackColorSpec>(back) | GetStyle<Win::StyleSpec>(style);
}
#endif

static const std::string_view GetAnsiEscapeCode(std::string& buf, EConsoleColor front, EConsoleColor back, EConsoleStyle style) SKR_NOEXCEPT
{
    buf.clear();
    const auto fcv = GetFrontColor<ANSI::FrontColorSpec>(front);
    const auto bcv = GetBackColor<ANSI::BackColorSpec>(back);
    const auto scv = GetStyle<ANSI::StyleSpec>(style);
    buf.append(fcv).append(bcv).append(scv);
    return buf;
}
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

void LogConsoleSink::sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT
{
    SkrZoneScopedN("ANSI::Print");

    // set color
    const auto L = static_cast<uint32_t>(event.get_level());
    const auto& ColorSet = color_sets_[L];
    const auto escape = GetAnsiEscapeCode(buf_cache_->buf, ColorSet.f, ColorSet.b, ColorSet.s);

    // output to console (use '\033[0m' to reset color)
    ::printf("%s%s\033[0m", escape.data(), (const char*)content.data());
}

void LogConsoleSink::flush() SKR_NOEXCEPT
{
    SkrZoneScopedN("ANSI::Flush");

    ::fflush(stdout);
}

void LogConsoleWindowSink::sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT
{
    SkrZoneScopedN("Console::Write");

#ifdef USE_WIN32_CONSOLE
    const auto StdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    // get origin
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    ::GetConsoleScreenBufferInfo(StdHandle, &csbiInfo);

    // set color
    const auto L = static_cast<uint32_t>(event.get_level());
    const auto attrs = (WORD)GetTextAttribute(color_sets_[L].f, color_sets_[L].b, color_sets_[L].s);
    if (csbiInfo.wAttributes != attrs)
        ::SetConsoleTextAttribute(StdHandle, attrs);

    // output to console
    ::WriteConsoleA(StdHandle, content.data(),
        static_cast<DWORD>(content.size()), nullptr, nullptr);

    // reset origin
    if (csbiInfo.wAttributes != attrs)
        ::SetConsoleTextAttribute(StdHandle, csbiInfo.wAttributes);
#else
    LogConsoleSink::sink(event, content);
#endif
}

void LogConsoleWindowSink::flush() SKR_NOEXCEPT
{
    SkrZoneScopedN("Console::Flush");

#ifdef USE_WIN32_CONSOLE
    const auto StdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::FlushFileBuffers(StdHandle);
#else
    LogConsoleSink::flush();
#endif
}

void LogDebugOutputSink::sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT
{
    SkrZoneScopedN("DebugOutput::Print");

    // set color
    const auto L = static_cast<uint32_t>(event.get_level());
    const auto& ColorSet = color_sets_[L];
    [[maybe_unused]] const auto escape = GetAnsiEscapeCode(buf_cache_->buf, ColorSet.f, ColorSet.b, ColorSet.s);

#ifdef USE_WIN32_CONSOLE
    skr::String output((const char8_t*)buf_cache_->buf.c_str());
    output.append(content);
    ::OutputDebugStringA(output.c_str_raw());
#else
    LogConsoleSink::sink(event, content);
#endif
}

void LogDebugOutputSink::flush() SKR_NOEXCEPT
{
    SkrZoneScopedN("DebugOutput::Flush");

#ifdef USE_WIN32_CONSOLE
    const auto StdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::FlushFileBuffers(StdHandle);
#else
    LogConsoleSink::flush();
#endif
}

struct CFILE
{
    CFILE(FILE* fp) SKR_NOEXCEPT : fp(fp) {}
    ~CFILE() SKR_NOEXCEPT { flush(); fclose(fp); }

    void write(const skr::StringView content)
    {
        SkrZoneScopedN("CFILE::write");
        fwrite(content.data(), sizeof(char8_t), content.size(), fp);
    }

    void flush() SKR_NOEXCEPT 
    { 
        SkrZoneScopedN("CFILE::flush");
        fflush(fp); 
    }
private:
    FILE* fp;
};

LogFileSink::LogFileSink() SKR_NOEXCEPT
    : LogSink(LogConstants::kDefaultFilePatternId)
{
    /*
    auto current_path = skr::filesystem::current_path();
    auto txt_path = current_path / "log.log";
    if (skr::filesystem::is_regular_file(txt_path))
    {
        auto time = skr::filesystem::last_write_time(txt_path);
        // append time to fname & rename 
        auto new_path = current_path / "log.txt";
        skr::filesystem::rename(txt_path, new_path);
    }
    */
    auto pname = skr::String(skr_get_current_process_name());
    pname.replace(u8".exe", u8"");
    auto fname = skr::format(u8"{}.log", pname);
    file_ = SkrNew<CFILE>(fopen(fname.c_str_raw(), "w"));
}

LogFileSink::~LogFileSink() SKR_NOEXCEPT
{
    SkrDelete(file_);
}

void LogFileSink::sink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT
{
    SkrZoneScopedN("FileSink::Print");

    file_->write(content);
}

void LogFileSink::flush() SKR_NOEXCEPT
{
    SkrZoneScopedN("FileSink::Flush");

    file_->flush();
}

} } // namespace skr::logging