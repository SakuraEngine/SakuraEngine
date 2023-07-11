#pragma once
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/misc/log/log_sink.hpp"
#include "log_worker.hpp"
#include "tscns.hpp"

#include "SkrRT/containers/hashmap.hpp"
#include <EASTL/unique_ptr.h>

namespace skr {
namespace log {

using LogPatternMap = skr::parallel_flat_hash_map<skr_guid_t, eastl::unique_ptr<LogPattern>, skr::guid::hash>;
using LogSinkMap = skr::parallel_flat_hash_map<skr_guid_t, eastl::unique_ptr<LogSink>, skr::guid::hash>;

struct RUNTIME_API LogManager
{
    static void Initialize() SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;

    static LogWorker* TryGetWorker() SKR_NOEXCEPT;
    static Logger* GetDefaultLogger() SKR_NOEXCEPT;

    static skr_guid_t RegisterPattern(eastl::unique_ptr<LogPattern> pattern);
    static bool RegisterPattern(skr_guid_t guid, eastl::unique_ptr<LogPattern> pattern);
    static LogPattern* QueryPattern(skr_guid_t guid);

    static skr_guid_t RegisterSink(eastl::unique_ptr<LogSink> sink);
    static bool RegisterSink(skr_guid_t guid, eastl::unique_ptr<LogSink> sink);
    static LogSink* QuerySink(skr_guid_t guid);

    static void PatternAndSink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT;
    static void FlushAllSinks() SKR_NOEXCEPT;
    static bool ShouldBacktrace(const LogEvent& event) SKR_NOEXCEPT;

    static SAtomic64 available_;
    static eastl::unique_ptr<LogWorker> worker_;
    static LogPatternMap patterns_;
    static LogSinkMap sinks_;
    static eastl::unique_ptr<skr::log::Logger> logger_;

    static TSCNS tscns_;
    static struct DateTime {
        void reset_date() SKR_NOEXCEPT;
        int64_t midnightNs;
        uint32_t year;
        uint32_t month;
        uint32_t day;
    } datetime_;
};

} } // namespace skr::log