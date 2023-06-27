#pragma once
#include "async/async_service.h"
#include "log_queue.hpp"
#include "containers/vector.hpp"
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
    std::once_flag start_once_flag;
    skr::SPtr<LogQueue> queue_;
    skr::vector<Logger*> loggers;
};

static const ServiceThreadDesc kLoggerWorkerThreadDesc =  {
    u8"AsyncLogWorker", SKR_THREAD_ABOVE_NORMAL
};

struct RUNTIME_API LogWorkerSingleton
{
    static LogWorker* TryGet() SKR_NOEXCEPT;

    static void Initialize() SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;

    static eastl::unique_ptr<LogWorker> _this;
    static SAtomic64 _available;
};

} } // namespace skr::log