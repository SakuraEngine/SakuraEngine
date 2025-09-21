#include "SkrProfile/profile.h"
#include "SkrContainersDef/hashmap.hpp"
#include "./log_manager.hpp"

namespace skr
{
namespace logging
{

std::once_flag default_logger_once_;
skr::UPtr<LogManagerImpl> LogManagerImpl::gLogManager = skr::UPtr<LogManagerImpl>::New();

LogManagerImpl::LogManagerImpl() SKR_NOEXCEPT
{

}

void LogManagerImpl::Initialize() SKR_NOEXCEPT
{
    tscns_.init();
    datetime_.reset_date();
    logger_ = skr::UPtr<skr::logging::Logger>::New(u8"Log");

    // register default pattern
    auto ret = RegisterPattern(LogConstants::kDefaultPatternId,
                u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message)");
    SKR_ASSERT(ret && "Default log pattern register failed!");

    // register default console pattern & sink
    ret = RegisterPattern(LogConstants::kDefaultConsolePatternId,
                          u8"[%(timestamp)][%(process_name)@%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
                          u8"\n    \x1b[90mIn %(function_name) At %(file_name):%(file_line)\x1b[0m");
    SKR_ASSERT(ret && "Default log console pattern register failed!");
    ret = RegisterSink(LogConstants::kDefaultConsoleSinkId, skr::UPtr<LogANSIOutputSink>::New());
    SKR_ASSERT(ret && "Default log console sink register failed!");

    // register default file pattern & sink
    ret = RegisterPattern(LogConstants::kDefaultFilePatternId,
                          u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
                          u8"\n    In %(function_name) At %(file_name):%(file_line)");
    SKR_ASSERT(ret && "Default log file pattern register failed!");

    /*
    ret = RegisterSink(LogConstants::kDefaultFileSinkId, skr::UPtr<LogFileSink>::New());
    SKR_ASSERT(ret && "Default log file sink register failed!");
    */
}
    
LogManager* LogManager::Get() SKR_NOEXCEPT
{
    static bool initialize_once = false;
    if (!initialize_once)
    {
        initialize_once = true;
        LogManagerImpl::gLogManager->Initialize();
    }
    return LogManagerImpl::gLogManager.get();
}

void LogManagerImpl::InitializeAsyncWorker() SKR_NOEXCEPT
{
    if (skr_atomic_load_acquire(&available_) != 0)
        return;

    // start worker
    if (!worker_)
    {
        worker_ = skr::UPtr<LogWorker>::New(kLoggerWorkerThreadDesc);
        worker_->run();
    }
    int64_t expected = 0;
    skr_atomic_compare_exchange_strong(&available_, &expected, 1ll);
}

void LogManagerImpl::FinalizeAsyncWorker() SKR_NOEXCEPT
{
    // skr::logging::LogManagerImpl::logger_.reset();
    if (skr_atomic_load_acquire(&available_) != 0)
    {
        worker_.reset();
        int64_t expected = 1;
        skr_atomic_compare_exchange_strong(&available_, &expected, 0ll);
    }
}

LogWorker* LogManagerImpl::TryGetWorker() SKR_NOEXCEPT
{
    if (skr_atomic_load_acquire(&available_) == 0)
        return nullptr;
    return worker_.get();
}

Logger* LogManagerImpl::GetDefaultLogger() SKR_NOEXCEPT
{
    return gLogManager->logger_.get();
}

GUID LogManagerImpl::RegisterPattern(const char8_t* pattern)
{
    auto guid = GUID::Create();
    patterns_.emplace(guid, skr::UPtr<LogPattern>::New(pattern));
    return guid;
}

bool LogManagerImpl::RegisterPattern(GUID guid, const char8_t* pattern)
{
    if (patterns_.find(guid) != patterns_.end())
        return false;
    patterns_.emplace(guid, skr::UPtr<LogPattern>::New(pattern));
    return true;
}

LogPattern* LogManagerImpl::QueryPattern(GUID guid)
{
    auto it = patterns_.find(guid);
    if (it != patterns_.end())
        return it->second.get();
    return nullptr;
}

GUID LogManagerImpl::RegisterSink(skr::UPtr<LogSink> sink)
{
    auto guid = GUID::Create();
    sinks_.emplace(guid, std::move(sink));
    return guid;
}

bool LogManagerImpl::RegisterSink(GUID guid, skr::UPtr<LogSink> sink)
{
    if (sinks_.find(guid) != sinks_.end())
        return false;
    sinks_.emplace(guid, std::move(sink));
    return true;
}

LogSink* LogManagerImpl::QuerySink(GUID guid)
{
    auto it = sinks_.find(guid);
    if (it != sinks_.end())
        return it->second.get();
    return nullptr;
}

void LogManagerImpl::PatternAndSink(const LogEvent& event, skr::StringView formatted_message) SKR_NOEXCEPT
{
    static thread_local skr::FlatHashMap<GUID, skr::String, skr::Hash<GUID>> patterns_set_;
    patterns_set_.clear();
    {
        SkrZoneScopedN("PatternAll");
        for (auto&& [id, sink] : sinks_)
        {
            auto   pattern_id = sink->get_pattern();
            auto&& iter       = patterns_set_.find(pattern_id);
            if (iter != patterns_set_.end())
                continue;

            if (auto p = LogManagerImpl::QueryPattern(pattern_id))
            {
                SkrZoneScopedN("LogPattern::Pattern");
                patterns_set_.insert({pattern_id, p->pattern(event, formatted_message)});
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
    }

    {
        SkrZoneScopedN("SinkAll");
        for (auto&& [id, sink] : sinks_)
        {
            auto pattern_id = sink->get_pattern();
            auto&& iter = patterns_set_.find(pattern_id);
            if (iter == patterns_set_.end())
                continue;
            
            if (auto p = LogManagerImpl::QueryPattern(pattern_id))
            {
                SkrZoneScopedN("LogSink::Sink");
                sink->sink(event, iter->second.view());
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
    }
    const bool bInplaceLog     = !LogManagerImpl::Get() || !gLogManager->TryGetWorker();
    const bool bFlushImmediate = LogConstants::gFlushBehavior == LogFlushBehavior::kFlushImmediate;
    if (bInplaceLog || bFlushImmediate)
    {
        FlushAllSinks();
    }
}

void LogManagerImpl::FlushAllSinks() SKR_NOEXCEPT
{
    SkrZoneScopedN("LogManagerImpl::FlushSinks");

    for (auto&& [id, sink] : sinks_)
    {
        sink->flush();
    }
}

bool LogManagerImpl::ShouldBacktrace(const LogEvent& event) SKR_NOEXCEPT
{
    const auto lv = event.get_level();
    const auto gt = lv >= LogLevel::kError;
    const auto ne = lv != LogLevel::kBackTrace;
    return gt && ne;
}

void LogManagerImpl::DateTime::reset_date() SKR_NOEXCEPT
{
    time_t     rawtime  = gLogManager->tscns_.rdns() / 1000000000;
    struct tm* timeinfo = ::localtime(&rawtime);
    timeinfo->tm_sec = timeinfo->tm_min = timeinfo->tm_hour = 0;
    midnightNs                                              = ::mktime(timeinfo) * 1000000000;
    year                                                    = 1900 + timeinfo->tm_year;
    month                                                   = 1 + timeinfo->tm_mon;
    day                                                     = timeinfo->tm_mday;
}

} // namespace logging
} // namespace skr
