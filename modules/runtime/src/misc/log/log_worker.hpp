#pragma once
#include "async/wait_timeout.hpp"
#include "async/async_service.h"
#include "containers/vector.hpp"
#include "misc/defer.hpp"

namespace skr {
namespace log {

struct Logger;
struct LogWorker : public AsyncService
{
    LogWorker(const ServiceThreadDesc& desc) SKR_NOEXCEPT
        : AsyncService(desc)
    {
    }

    ~LogWorker() SKR_NOEXCEPT
    {
        drain();
        if (get_status() == skr::ServiceThread::Status::kStatusRunning)
        {
            setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
            stop();
        }
        wait_stop();
        exit();
    }

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

    skr::vector<Logger*> loggers;
};

struct RUNTIME_API LogWorkerSingleton
{
    static LogWorker* Get() SKR_NOEXCEPT
    {
        return &_this;
    }
    static LogWorker _this;
};

} } // namespace skr::log