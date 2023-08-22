#include "misc/log/log_manager.hpp"

#include <EASTL/fixed_hash_set.h>

#include "SkrProfile/profile.h"

namespace skr {
namespace log {

std::once_flag default_logger_once_;

LogManager::LogManager() SKR_NOEXCEPT
{

}

void LogManager::Initialize() SKR_NOEXCEPT
{
    tscns_.init();
    datetime_.reset_date();
    logger_ = eastl::make_unique<skr::log::Logger>(u8"Log");

    // register default pattern
    auto ret = RegisterPattern(LogConstants::kDefaultPatternId, 
        eastl::make_unique<LogPattern>(
            u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message)"
        ));
    SKR_ASSERT(ret && "Default log pattern register failed!");
    
    // register default console pattern & sink
    ret = RegisterPattern(LogConstants::kDefaultConsolePatternId, 
        eastl::make_unique<LogPattern>(
            u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
            u8"\n    \x1b[90mIn %(function_name) At %(file_name):%(file_line)\x1b[0m"
        ));
    SKR_ASSERT(ret && "Default log console pattern register failed!");
    ret = RegisterSink(LogConstants::kDefaultConsoleSinkId, eastl::make_unique<LogANSIOutputSink>());
    SKR_ASSERT(ret && "Default log console sink register failed!");
    
    // register default file pattern & sink
    ret = RegisterPattern(LogConstants::kDefaultFilePatternId, 
        eastl::make_unique<LogPattern>(
            u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
            u8"\n    In %(function_name) At %(file_name):%(file_line)"
        ));
    SKR_ASSERT(ret && "Default log file pattern register failed!");
    ret = RegisterSink(LogConstants::kDefaultFileSinkId, eastl::make_unique<LogFileSink>());
    SKR_ASSERT(ret && "Default log file sink register failed!");
}

LogManager* LogManager::Get() SKR_NOEXCEPT
{
    static eastl::unique_ptr<LogManager> manager = eastl::make_unique<LogManager>();
    static bool initialize_once = false;
    if (!initialize_once)
    {
        initialize_once = true;
        manager->Initialize();
    }
    return manager.get();
}

void LogManager::InitializeAsyncWorker() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) != 0)
        return;

    // start worker
    if (!worker_)
    {
        worker_ = eastl::make_unique<LogWorker>(kLoggerWorkerThreadDesc);
        worker_->run();
    }
    skr_atomic64_cas_relaxed(&available_, 0, 1);
}

void LogManager::FinalizeAsyncWorker() SKR_NOEXCEPT
{
    // skr::log::LogManager::logger_.reset();
    if (skr_atomic64_load_acquire(&available_) != 0)
    {
        worker_.reset();
        skr_atomic64_cas_relaxed(&available_, 1, 0);
    }
}

LogWorker* LogManager::TryGetWorker() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) == 0)
        return nullptr;
    return worker_.get();
}

Logger* LogManager::GetDefaultLogger() SKR_NOEXCEPT
{
    auto Manager = skr::log::LogManager::Get();
    return Manager->logger_.get();
}

skr_guid_t LogManager::RegisterPattern(eastl::unique_ptr<LogPattern> pattern)
{
    auto guid = skr_guid_t();
    skr_make_guid(&guid);
    patterns_.emplace(guid, skr::move(pattern));
    return guid;
}

bool LogManager::RegisterPattern(skr_guid_t guid, eastl::unique_ptr<LogPattern> pattern)
{
    if (patterns_.find(guid) != patterns_.end())
        return false;
    patterns_.emplace(guid, skr::move(pattern));
    return true;
}

LogPattern* LogManager::QueryPattern(skr_guid_t guid)
{
    auto it = patterns_.find(guid);
    if (it != patterns_.end())
        return it->second.get();
    return nullptr;
}

skr_guid_t LogManager::RegisterSink(eastl::unique_ptr<LogSink> sink)
{
    auto guid = skr_guid_t();
    skr_make_guid(&guid);
    sinks_.emplace(guid, skr::move(sink));
    return guid;
}

bool LogManager::RegisterSink(skr_guid_t guid, eastl::unique_ptr<LogSink> sink)
{
    if (sinks_.find(guid) != sinks_.end())
        return false;
    sinks_.emplace(guid, skr::move(sink));
    return true;
}

LogSink* LogManager::QuerySink(skr_guid_t guid)
{
    auto it = sinks_.find(guid);
    if (it != sinks_.end())
        return it->second.get();
    return nullptr;
}

void LogManager::PatternAndSink(const LogEvent& event, skr::string_view formatted_message) SKR_NOEXCEPT
{
    eastl::fixed_hash_set<skr_guid_t, 4, 5, true, skr::guid::hash> patterns_;
    {
        SkrZoneScopedN("PatternAll");
        for (auto&& [id, sink] : sinks_)
        {
            auto pattern_id = sink->get_pattern();
            auto&& iter = patterns_.find(pattern_id);
            if (iter != patterns_.end())
                continue;
            
            if (auto p = LogManager::QueryPattern(pattern_id))
            {
                SkrZoneScopedN("LogPattern::Pattern");

                [[maybe_unused]] 
                auto& _ = p->pattern(event, formatted_message);
                patterns_.insert(pattern_id);
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

            if (auto p = LogManager::QueryPattern(pattern_id))
            {
                SkrZoneScopedN("LogSink::Sink");

                sink->sink(event, p->last_result().view());
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
    }
    if (LogConstants::gFlushBehavior == LogFlushBehavior::kFlushImmediate)
    {
        FlushAllSinks();
    }
}

void LogManager::FlushAllSinks() SKR_NOEXCEPT
{
    SkrZoneScopedN("LogManager::FlushSinks");

    for (auto&& [id, sink] : sinks_)
    {
        sink->flush();
    }
}

bool LogManager::ShouldBacktrace(const LogEvent& event) SKR_NOEXCEPT
{
    const auto lv = event.get_level();
    const auto gt = lv >= LogLevel::kError;
    const auto ne = lv != LogLevel::kBackTrace;
    return gt && ne;
}

void LogManager::DateTime::reset_date() SKR_NOEXCEPT
{
    auto Manager = LogManager::Get();
    time_t rawtime = Manager->tscns_.rdns() / 1000000000;
    struct tm* timeinfo = ::localtime(&rawtime);
    timeinfo->tm_sec = timeinfo->tm_min = timeinfo->tm_hour = 0;
    midnightNs = ::mktime(timeinfo) * 1000000000;
    year = 1900 + timeinfo->tm_year;
    month = 1 + timeinfo->tm_mon;
    day = timeinfo->tm_mday;
}

} } // namespace skr::log
