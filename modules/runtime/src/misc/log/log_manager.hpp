#pragma once
#include "platform/guid.hpp"
#include "log_worker.hpp"
#include "tscns.hpp"

#include "containers/hashmap.hpp"
#include <EASTL/unique_ptr.h>

namespace skr {
namespace log {

using LogPatternMap = skr::parallel_flat_hash_map<skr_guid_t, eastl::unique_ptr<LogPattern>, skr::guid::hash>;

struct RUNTIME_API LogManager
{
    static void Initialize() SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;

    static LogWorker* TryGetWorker() SKR_NOEXCEPT;
    static Logger* GetDefaultLogger() SKR_NOEXCEPT;

    static skr_guid_t RegisterPattern(eastl::unique_ptr<LogPattern> pattern);
    static LogPattern* QueryPattern(skr_guid_t guid);

    static SAtomic64 available_;
    static eastl::unique_ptr<LogWorker> worker_;
    static LogPatternMap patterns_;
    static eastl::unique_ptr<skr::log::Logger> logger_;
    static TSCNS tscns_;
};

} } // namespace skr::log