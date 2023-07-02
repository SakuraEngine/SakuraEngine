#include "../../pch.hpp"
#include "misc/log/log_manager.hpp"

#include <EASTL/fixed_hash_set.h>

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {

SAtomic64 LogManager::available_ = 0;
eastl::unique_ptr<LogWorker> LogManager::worker_ = nullptr;
LogPatternMap LogManager::patterns_ = {};
LogSinkMap LogManager::sinks_ = {};
std::once_flag default_logger_once_;
std::once_flag default_pattern_once_;
eastl::unique_ptr<skr::log::Logger> LogManager::logger_ = nullptr;
TSCNS LogManager::tscns_ = {};
LogManager::DateTime LogManager::datetime_ = {};

void LogManager::Initialize() SKR_NOEXCEPT
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

void LogManager::Finalize() SKR_NOEXCEPT
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
    std::call_once(
        skr::log::default_logger_once_,
        [] {
            skr::log::LogManager::tscns_.init();
            skr::log::LogManager::datetime_.reset_date();
            skr::log::LogManager::logger_ = eastl::make_unique<skr::log::Logger>(u8"Log");

            // register default pattern
            auto ret = LogManager::RegisterPattern(LogConstants::kDefaultPatternId, 
                eastl::make_unique<LogPattern>(
                    u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message)"
                ));
            SKR_ASSERT(ret && "Default log pattern register failed!");
            
            // register default console pattern & sink
            ret = LogManager::RegisterPattern(LogConstants::kDefaultConsolePatternId, 
                eastl::make_unique<LogPattern>(
                    u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
                    u8"\n    \x1b[90mIn %(function_name) At %(file_name):%(file_line)\x1b[0m"
                ));
            SKR_ASSERT(ret && "Default log console pattern register failed!");
            ret = LogManager::RegisterSink(LogConstants::kDefaultConsoleSinkId, eastl::make_unique<LogConsoleSink>());
            SKR_ASSERT(ret && "Default log console sink register failed!");
            
            // register default file pattern & sink
            ret = LogManager::RegisterPattern(LogConstants::kDefaultFilePatternId, 
                eastl::make_unique<LogPattern>(
                    u8"[%(timestamp)][%(thread_name)(tid:%(thread_id))] %(logger_name).%(level_name): %(message) "
                    u8"\n    In %(function_name) At %(file_name):%(file_line)"
                ));
            SKR_ASSERT(ret && "Default log file pattern register failed!");
            ret = LogManager::RegisterSink(LogConstants::kDefaultFileSinkId, eastl::make_unique<LogFileSink>());
            SKR_ASSERT(ret && "Default log file sink register failed!");
        }
    );
    return logger_.get();
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
        ZoneScopedN("PatternAll");
        for (auto&& [id, sink] : sinks_)
        {
            auto pattern_id = sink->get_pattern();
            auto&& iter = patterns_.find(pattern_id);
            if (iter != patterns_.end())
                continue;
            
            if (auto p = LogManager::QueryPattern(pattern_id))
            {
                ZoneScopedN("LogPattern::Pattern");

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
        ZoneScopedN("SinkAll");
        for (auto&& [id, sink] : sinks_)
        {
            auto pattern_id = sink->get_pattern();

            if (auto p = LogManager::QueryPattern(pattern_id))
            {
                ZoneScopedN("LogSink::Sink");

                sink->sink(event, p->last_result().view());
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
    }
}

void LogManager::DateTime::reset_date() SKR_NOEXCEPT
{
    time_t rawtime = LogManager::tscns_.rdns() / 1000000000;
    struct tm* timeinfo = ::localtime(&rawtime);
    timeinfo->tm_sec = timeinfo->tm_min = timeinfo->tm_hour = 0;
    midnightNs = ::mktime(timeinfo) * 1000000000;
    year = 1900 + timeinfo->tm_year;
    month = 1 + timeinfo->tm_mon;
    day = timeinfo->tm_mday;
}

} } // namespace skr::log
