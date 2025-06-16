#include "SkrCore/log.h"
#include "SkrCore/log/logger.hpp"
#include "./log_manager.hpp"

#include <thread>
#include <csignal>
#include <stdarg.h> // va_start
#include <stdio.h> // ::atexit

#include "SkrProfile/profile.h"

namespace skr {
namespace logging {
    
using namespace skr::literals;
const char* kLogMemoryName = "sakura::log";
skr::logging::LogLevel LogConstants::gLogLevel = skr::logging::LogLevel::kTrace;
skr::logging::LogFlushBehavior LogConstants::gFlushBehavior = skr::logging::LogFlushBehavior::kAuto;
const skr_guid_t LogConstants::kDefaultPatternId = u8"c236a30a-c91e-4b26-be7c-c7337adae428"_guid;
const skr_guid_t LogConstants::kDefaultConsolePatternId = u8"e3b22b5d-95ea-462d-93bf-b8b91e7b991b"_guid;
const skr_guid_t LogConstants::kDefaultConsoleSinkId = u8"11b910c7-de4b-4bba-9dee-5853f35b0c10"_guid;
const skr_guid_t LogConstants::kDefaultFilePatternId = u8"75871d37-ba78-4a75-bb7a-455f08bc8a2e"_guid;
const skr_guid_t LogConstants::kDefaultFileSinkId = u8"289d0408-dec8-4ceb-ae32-55d4793df983"_guid;

LogEvent::LogEvent(Logger* logger, LogLevel level, const LogSourceData& src_data) SKR_NOEXCEPT
    : level(level), timestamp(LogManagerImpl::gLogManager->tscns_.rdtsc()), 
      thread_id(skr_current_thread_id()), thread_name(skr_current_thread_get_name()),
      logger(logger), src_data(src_data)
{

}

LogFormatter::~LogFormatter() SKR_NOEXCEPT
{

}

skr::String const& LogFormatter::format(const skr::String& format, const ArgsList& args_list)
{
    args_list.format_(format, *this);
    return formatted_string;
}

Logger::Logger(const char8_t* name) SKR_NOEXCEPT
    : name(name)
{
    if (auto worker = LogManagerImpl::gLogManager->TryGetWorker())
    {
        worker->add_logger(this);
    }
}

Logger::~Logger() SKR_NOEXCEPT
{
    if (auto manager = LogManagerImpl::gLogManager.get())
    {
        if (auto worker = manager->TryGetWorker())
        {
            worker->remove_logger(this);
        }
    }
}

Logger* Logger::GetDefault() SKR_NOEXCEPT
{
    return LogManagerImpl::gLogManager->GetDefaultLogger();
}

void Logger::onLog(const LogEvent& ev) SKR_NOEXCEPT
{
    if (auto should_backtrace = LogManagerImpl::gLogManager->ShouldBacktrace(ev))
    {
        skr_log_flush();
    }
}

void Logger::sinkDefaultImmediate(const LogEvent& e, skr::StringView what) const SKR_NOEXCEPT
{
    skr::logging::LogManagerImpl::gLogManager->PatternAndSink(e, what);
}

bool Logger::canPushToQueue() const SKR_NOEXCEPT
{
    auto worker = LogManagerImpl::gLogManager->TryGetWorker();
    return worker;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::StringView format, ArgsList&& args_list) SKR_NOEXCEPT
{
    auto worker = LogManagerImpl::gLogManager->TryGetWorker();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, format, std::move(args_list), ev.get_level() == LogLevel::kBackTrace);
        notifyWorker();
        return true;
    }
    return false;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::String&& what) SKR_NOEXCEPT
{
    auto worker = LogManagerImpl::gLogManager->TryGetWorker();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, std::move(what), ev.get_level() == LogLevel::kBackTrace);
        notifyWorker();
        return true;
    }
    return false;
}

void Logger::notifyWorker() SKR_NOEXCEPT
{
    if (auto worker = LogManagerImpl::gLogManager->TryGetWorker())
    {
        worker->awake();
    }
}

} } // namespace skr::logging

SKR_EXTERN_C
void skr_log_set_level(int level)
{
    const auto kLogLevel = skr::logging::LogConstants::kLogLevelsLUT[level];
    skr::logging::LogConstants::gLogLevel = kLogLevel;
}

SKR_EXTERN_C 
void skr_log_set_flush_behavior(int behavior)
{
    const auto kLogBehavior = skr::logging::LogConstants::kFlushBehaviorLUT[behavior];
    skr::logging::LogConstants::gFlushBehavior = kLogBehavior;
}

SKR_EXTERN_C 
void skr_log_log(int level, const char* file, const char* func, const char* line, const char8_t* fmt, ...)
{
    SkrZoneScopedN("Log");
    
    const auto kLogLevel = skr::logging::LogConstants::kLogLevelsLUT[level];
    if (kLogLevel < skr::logging::LogConstants::gLogLevel) return;

    auto logger = skr::logging::LogManagerImpl::gLogManager->GetDefaultLogger();
    const skr::logging::LogSourceData Src = { file, func, line  };
    const auto Event = skr::logging::LogEvent(logger, kLogLevel, Src);

    va_list va_args;
    va_start(va_args, fmt);
    logger->log(Event, fmt, va_args);
    va_end(va_args);
}

SKR_EXTERN_C 
void skr_log_finalize_async_worker()
{
    {
        if (auto worker = skr::logging::LogManagerImpl::gLogManager->TryGetWorker())
            worker->drain();
    }
    skr::logging::LogManagerImpl::gLogManager->FinalizeAsyncWorker();

    auto worker = skr::logging::LogManagerImpl::gLogManager->TryGetWorker();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}