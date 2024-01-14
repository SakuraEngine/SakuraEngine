#pragma once
#include "SkrGuid/guid.hpp"
#include "SkrCore/log/log_sink.hpp"
#include "SkrCore/log/log_pattern.hpp"
#include "SkrContainers/hashmap.hpp"
#include "log_worker.hpp"
#include "tscns.hpp"

#include "SkrContainers/deprecated.hpp"

namespace skr {
namespace log {

using LogPatternMap = skr::ParallelFlatHashMap<skr_guid_t, skr::unique_ptr<LogPattern>, skr::guid::hash>;
using LogSinkMap = skr::ParallelFlatHashMap<skr_guid_t, skr::unique_ptr<LogSink>, skr::guid::hash>;

struct SKR_CORE_API LogManager
{
    LogManager() SKR_NOEXCEPT;
    static LogManager* Get() SKR_NOEXCEPT;

    void Initialize() SKR_NOEXCEPT;
    void InitializeAsyncWorker() SKR_NOEXCEPT;
    void FinalizeAsyncWorker() SKR_NOEXCEPT;

    LogWorker* TryGetWorker() SKR_NOEXCEPT;
    Logger* GetDefaultLogger() SKR_NOEXCEPT;

    skr_guid_t RegisterPattern(skr::unique_ptr<LogPattern> pattern);
    bool RegisterPattern(skr_guid_t guid, skr::unique_ptr<LogPattern> pattern);
    LogPattern* QueryPattern(skr_guid_t guid);

    skr_guid_t RegisterSink(skr::unique_ptr<LogSink> sink);
    bool RegisterSink(skr_guid_t guid, skr::unique_ptr<LogSink> sink);
    LogSink* QuerySink(skr_guid_t guid);

    void PatternAndSink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT;
    void FlushAllSinks() SKR_NOEXCEPT;
    bool ShouldBacktrace(const LogEvent& event) SKR_NOEXCEPT;

    SAtomic64 available_ = 0;
    skr::unique_ptr<LogWorker> worker_ = nullptr;
    LogPatternMap patterns_ = {};
    LogSinkMap sinks_ = {};
    skr::unique_ptr<skr::log::Logger> logger_ = nullptr;

    TSCNS tscns_ = {};
    struct DateTime {
        void reset_date() SKR_NOEXCEPT;
        int64_t midnightNs = 0;
        uint32_t year = 0;
        uint32_t month = 0;
        uint32_t day = 0;
    } datetime_ = {};
};

} } // namespace skr::log