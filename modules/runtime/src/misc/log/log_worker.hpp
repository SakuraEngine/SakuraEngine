#pragma once
#include "SkrRT/async/async_service.h"
#include "SkrRT/misc/log/log_pattern.hpp"
#include "log_queue.hpp"

#include "SkrRT/containers/vector.hpp"
#include "SkrRT/containers/sptr.hpp"

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
    void flush(SThreadID tid) SKR_NOEXCEPT;
    // void flush() SKR_NOEXCEPT;

    virtual skr::AsyncResult serve() SKR_NOEXCEPT;

protected:
    void patternAndSink(const LogElement& e) SKR_NOEXCEPT;

    friend struct Logger;
    skr::SPtr<LogQueue> queue_;
    skr::vector<Logger*> loggers_;
    LogFormatter formatter_;
};

static const ServiceThreadDesc kLoggerWorkerThreadDesc =  {
    u8"AsyncLogWorker", SKR_THREAD_ABOVE_NORMAL
};

} } // namespace skr::log