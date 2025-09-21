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

using LogPatternMap = skr::ParallelFlatHashMap<GUID, skr::UPtr<LogPattern>, skr::Hash<GUID>>;
using LogSinkMap    = skr::ParallelFlatHashMap<GUID, skr::UPtr<LogSink>, skr::Hash<GUID>>;

struct SKR_CORE_API LogManagerImpl : public LogManager 
{
    LogManagerImpl() SKR_NOEXCEPT;

public:
    void InitializeAsyncWorker() SKR_NOEXCEPT override;
    void FinalizeAsyncWorker() SKR_NOEXCEPT override;
    void FlushAllSinks() SKR_NOEXCEPT override;

    GUID RegisterPattern(const char8_t* pattern) override;;
    bool RegisterPattern(GUID guid, const char8_t* pattern) override;

public:
    void Initialize() SKR_NOEXCEPT;
    LogWorker* TryGetWorker() SKR_NOEXCEPT;
    Logger*    GetDefaultLogger() SKR_NOEXCEPT;

    LogPattern* QueryPattern(GUID guid);

    GUID RegisterSink(skr::UPtr<LogSink> sink) override;
    bool       RegisterSink(GUID guid, skr::UPtr<LogSink> sink) override;
    LogSink*   QuerySink(GUID guid) override;

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