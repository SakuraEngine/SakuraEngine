#pragma once
#include "platform/guid.hpp"
#include "platform/time.h"
#include "async/async_service.h"
#include "misc/log/log_pattern.hpp"
#include "log_queue.hpp"

#include "containers/vector.hpp"
#include "containers/sptr.hpp"
#include "containers/hashmap.hpp"
#include <EASTL/unique_ptr.h>

namespace skr {
namespace log {

struct Logger;
struct LogWorker : public AsyncService
{
    LogWorker(const ServiceThreadDesc& desc) SKR_NOEXCEPT;
    ~LogWorker() SKR_NOEXCEPT;

    void drain()
    {
        while (predicate())
        {
            for (uint32_t i = 0; i < 64; i++)
                skr_thread_sleep(0);
        }
    }

    void add_logger(Logger* logger) SKR_NOEXCEPT;
    void remove_logger(Logger* logger) SKR_NOEXCEPT;

    bool predicate() SKR_NOEXCEPT;
    void process_logs() SKR_NOEXCEPT;
    virtual skr::AsyncResult serve() SKR_NOEXCEPT;

protected:
    friend struct Logger;
    skr::SPtr<LogQueue> queue_;
    skr::vector<Logger*> loggers_;
    LogFormatter formatter_;
};

static const ServiceThreadDesc kLoggerWorkerThreadDesc =  {
    u8"AsyncLogWorker", SKR_THREAD_ABOVE_NORMAL
};

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
};

} } // namespace skr::log