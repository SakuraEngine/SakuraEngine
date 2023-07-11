#include "../../pch.hpp"
#include "SkrRT/misc/log.h"
#include "SkrRT/misc/log/logger.hpp"
#include "SkrRT/misc/log/log_manager.hpp"

#include <thread>
#include <csignal>
#include <stdarg.h> // va_start
#include <stdio.h> // ::atexit

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {
    
using namespace skr::guid::literals;
const char* kLogMemoryName = "sakura::log";
skr::log::LogLevel LogConstants::gLogLevel = skr::log::LogLevel::kTrace;
skr::log::LogFlushBehavior LogConstants::gFlushBehavior = skr::log::LogFlushBehavior::kAuto;
const skr_guid_t LogConstants::kDefaultPatternId = u8"c236a30a-c91e-4b26-be7c-c7337adae428"_guid;
const skr_guid_t LogConstants::kDefaultConsolePatternId = u8"e3b22b5d-95ea-462d-93bf-b8b91e7b991b"_guid;
const skr_guid_t LogConstants::kDefaultConsoleSinkId = u8"11b910c7-de4b-4bba-9dee-5853f35b0c10"_guid;
const skr_guid_t LogConstants::kDefaultFilePatternId = u8"75871d37-ba78-4a75-bb7a-455f08bc8a2e"_guid;
const skr_guid_t LogConstants::kDefaultFileSinkId = u8"289d0408-dec8-4ceb-ae32-55d4793df983"_guid;

LogEvent::LogEvent(Logger* logger, LogLevel level, const LogSourceData& src_data) SKR_NOEXCEPT
    : level(level), timestamp(LogManager::tscns_.rdtsc()), 
      thread_id(skr_current_thread_id()), thread_name(skr_current_thread_get_name()),
      logger(logger), src_data(src_data)
{

}

LogFormatter::~LogFormatter() SKR_NOEXCEPT
{

}

skr::string const& LogFormatter::format(const skr::string& format, const ArgsList& args_list)
{
    args_list.format_(format, *this);
    return formatted_string;
}

Logger::Logger(const char8_t* name) SKR_NOEXCEPT
    : name(name)
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->add_logger(this);
    }
}

Logger::~Logger() SKR_NOEXCEPT
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->remove_logger(this);
    }
}

Logger* Logger::GetDefault() SKR_NOEXCEPT
{
    return LogManager::GetDefaultLogger();
}

void Logger::onLog(const LogEvent& ev) SKR_NOEXCEPT
{
    if (auto should_backtrace = LogManager::ShouldBacktrace(ev))
    {
        skr_log_flush();
    }
}

void Logger::sinkDefaultImmediate(const LogEvent& e, skr::string_view what) const SKR_NOEXCEPT
{
    skr::log::LogManager::PatternAndSink(e, what);
}

bool Logger::canPushToQueue() const SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    return worker;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string_view format, ArgsList&& args_list) SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, format, skr::move(args_list), ev.get_level() == LogLevel::kBackTrace);
        notifyWorker();
        return true;
    }
    return false;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string&& what) SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, skr::move(what), ev.get_level() == LogLevel::kBackTrace);
        notifyWorker();
        return true;
    }
    return false;
}

void Logger::notifyWorker() SKR_NOEXCEPT
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->awake();
    }
}

} } // namespace skr::log

RUNTIME_EXTERN_C
void skr_log_set_level(int level)
{
    const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];
    skr::log::LogConstants::gLogLevel = kLogLevel;
}

RUNTIME_EXTERN_C 
void skr_log_set_flush_behavior(int behavior)
{
    const auto kLogBehavior = skr::log::LogConstants::kFlushBehaviorLUT[behavior];
    skr::log::LogConstants::gFlushBehavior = kLogBehavior;
}

RUNTIME_EXTERN_C 
void skr_log_log(int level, const char* file, const char* func, const char* line, const char* fmt, ...)
{
    ZoneScopedN("Log");
    
    const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];
    if (kLogLevel < skr::log::LogConstants::gLogLevel) return;

    auto logger = skr::log::LogManager::GetDefaultLogger();
    const skr::log::LogSourceData Src = { file, func, line  };
    const auto Event = skr::log::LogEvent(logger, kLogLevel, Src);

    va_list va_args;
    va_start(va_args, fmt);
    logger->log(Event, (const char8_t*)fmt, va_args);
    va_end(va_args);
}

RUNTIME_EXTERN_C 
void skr_log_finalize_async_worker()
{
    {
        if (auto worker = skr::log::LogManager::TryGetWorker())
            worker->drain();
    }
    skr::log::LogManager::Finalize();

    auto worker = skr::log::LogManager::TryGetWorker();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}