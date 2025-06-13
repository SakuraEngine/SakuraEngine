#pragma once
#include "SkrCore/log.hpp"
#include "SkrCore/log/log_sink.hpp"
#include "SkrCore/log/log_pattern.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "log_worker.hpp"
#include "tscns.hpp"

namespace skr
{
namespace logging
{

using LogPatternMap = skr::ParallelFlatHashMap<skr_guid_t, skr::UPtr<LogPattern>, skr::Hash<skr_guid_t>>;
using LogSinkMap    = skr::ParallelFlatHashMap<skr_guid_t, skr::UPtr<LogSink>, skr::Hash<skr_guid_t>>;

struct SKR_CORE_API LogManagerImpl : public LogManager 
{
    LogManagerImpl() SKR_NOEXCEPT;

public:
    void InitializeAsyncWorker() SKR_NOEXCEPT override;
    void FinalizeAsyncWorker() SKR_NOEXCEPT override;
    void FlushAllSinks() SKR_NOEXCEPT override;

    skr_guid_t RegisterPattern(const char8_t* pattern) override;;
    bool RegisterPattern(skr_guid_t guid, const char8_t* pattern) override;

public:
    void Initialize() SKR_NOEXCEPT;
    LogWorker* TryGetWorker() SKR_NOEXCEPT;
    Logger*    GetDefaultLogger() SKR_NOEXCEPT;

    LogPattern* QueryPattern(skr_guid_t guid);

    skr_guid_t RegisterSink(skr::UPtr<LogSink> sink) override;
    bool       RegisterSink(skr_guid_t guid, skr::UPtr<LogSink> sink) override;
    LogSink*   QuerySink(skr_guid_t guid) override;

    void PatternAndSink(const LogEvent& event, skr::StringView content) SKR_NOEXCEPT;
    bool ShouldBacktrace(const LogEvent& event) SKR_NOEXCEPT;

    SAtomic64                         available_ = 0;
    skr::UPtr<LogWorker>        worker_    = nullptr;
    LogPatternMap                     patterns_  = {};
    LogSinkMap                        sinks_     = {};
    skr::UPtr<skr::logging::Logger> logger_    = nullptr;
    static skr::UPtr<LogManagerImpl> gLogManager;

    TSCNS tscns_ = {};
    struct DateTime {
        void     reset_date() SKR_NOEXCEPT;
        int64_t  midnightNs = 0;
        uint32_t year       = 0;
        uint32_t month      = 0;
        uint32_t day        = 0;
    } datetime_ = {};
};

} // namespace logging
} // namespace skr