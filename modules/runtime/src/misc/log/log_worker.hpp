#pragma once
#include "async/wait_timeout.hpp"
#include "async/async_service.h"
#include "log_queue.hpp"
#include "containers/vector.hpp"
#include "misc/defer.hpp"

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
    u8"logger_worker", SKR_THREAD_ABOVE_NORMAL
};

struct RUNTIME_API LogWorkerSingleton
{
    static SPtr<LogWorker> ShareOrCreate() SKR_NOEXCEPT;
    static SWeakPtr<LogWorker> _weak_this;
};

} } // namespace skr::log