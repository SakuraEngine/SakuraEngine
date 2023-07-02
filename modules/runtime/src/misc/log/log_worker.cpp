#include "../../pch.hpp"
#include "misc/log.h"
#include "misc/log/logger.hpp"
#include "misc/log/log_manager.hpp"

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {

LogWorker::LogWorker(const ServiceThreadDesc& desc) SKR_NOEXCEPT
    : AsyncService(desc), queue_(SPtr<LogQueue>::Create())
{

}

LogWorker::~LogWorker() SKR_NOEXCEPT
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

void LogWorker::add_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers_.emplace_back(logger);
}

void LogWorker::remove_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers_.erase(std::remove(loggers_.begin(), loggers_.end(), logger), loggers_.end());
}

bool LogWorker::predicate() SKR_NOEXCEPT
{
    return queue_->query_cnt();
}

void LogWorker::process_logs() SKR_NOEXCEPT
{
    LogManager::tscns_.calibrate();
    
    LogElement e;
    while (queue_->try_dequeue(e))
    {
        ZoneScopedNC("LogSingle", tracy::Color::Orchid1);
        const auto& what = e.need_format ? 
            formatter_.format(e.format, e.args) :
            e.format;
        skr::log::LogManager::PatternAndSink(e.event, what.view());
    }
}

skr::AsyncResult LogWorker::serve() SKR_NOEXCEPT
{
    if (!predicate())
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_SLEEPING);
        sleep();
        return ASYNC_RESULT_OK;
    }
    
    {
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_RUNNING);
        ZoneScopedNC("Dispatch", tracy::Color::Orchid3);
        process_logs();
    }
    return ASYNC_RESULT_OK;
}

} } // namespace skr::log

RUNTIME_EXTERN_C
void log_initialize_async_worker()
{
    skr::log::LogManager::Initialize();

    auto worker = skr::log::LogManager::TryGetWorker();
    SKR_ASSERT(worker && "worker must not be null & something is wrong with initialization!");

    ::atexit(+[]() {
        auto worker = skr::log::LogManager::TryGetWorker();
        SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
    });
}
