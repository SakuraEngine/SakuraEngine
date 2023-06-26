#include "log_queue.hpp"
#include "log_worker.hpp"
#include "logger.hpp"

#include <thread>
#include <csignal>
#include <stdio.h> // ::atexit

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {

Logger::Logger() SKR_NOEXCEPT
{
    auto worker = LogWorkerSingleton::ShareOrCreate();
    queue_ = worker->queue_;
    worker_ = worker;
    worker->add_logger(this);
}

Logger::~Logger() SKR_NOEXCEPT
{
    worker_->remove_logger(this);
}

void Logger::notifyWorker() SKR_NOEXCEPT
{
    worker_->awake();
}

SWeakPtr<LogWorker> LogWorkerSingleton::_weak_this;
std::mutex g_worker_shared_mutex;
SPtr<LogWorker> LogWorkerSingleton::ShareOrCreate() SKR_NOEXCEPT
{
    std::lock_guard _(g_worker_shared_mutex);
    if (auto _this = _weak_this.lock())
    {
        return _this;
    }
    auto new_this = SPtr<LogWorker>::Create(kLoggerWorkerThreadDesc);
    _weak_this = new_this;
    new_this->run();
    return new_this;
}

static const skr::log::LogLevel kLogLevelsLUT[] = {
    skr::log::LogLevel::kTrace, 
    skr::log::LogLevel::kDebug, 
    skr::log::LogLevel::kInfo, 
    skr::log::LogLevel::kWarning, 
    skr::log::LogLevel::kError, 
    skr::log::LogLevel::kFatal 
};
static_assert(sizeof(kLogLevelsLUT) / sizeof(kLogLevelsLUT[0]) == (int)skr::log::LogLevel::kCount, "kLogLevelsLUT size mismatch");
skr::log::LogLevel g_log_level = skr::log::LogLevel::kTrace;
skr::SPtr<skr::log::Logger> g_logger = {};
std::once_flag g_start_once_flag;

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
    loggers.emplace_back(logger);
}

void LogWorker::remove_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers.erase(std::remove(loggers.begin(), loggers.end(), logger), loggers.end());
}

bool LogWorker::predicate() SKR_NOEXCEPT
{
    return queue_->query_cnt();
}

void LogWorker::process_logs() SKR_NOEXCEPT
{
    LogQueueElement e;
    while (queue_->try_dequeue(e))
    {
        ZoneScopedNC("LogSingle", tracy::Color::Orchid1);
        const auto what = e.produce();
        printf("%s\n", what.c_str());
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
void log_set_lock(log_LockFn fn, void* udata)
{
    // thread-safe and dont need this
}

RUNTIME_EXTERN_C
void log_set_level(int level)
{
    const auto kLogLevel = skr::log::kLogLevelsLUT[level];\
    skr::log::g_log_level = kLogLevel;
}

RUNTIME_EXTERN_C 
void log_log(int level, const char* file, int line, const char* fmt, ...)
{
    const auto kLogLevel = skr::log::kLogLevelsLUT[level];
    if (kLogLevel < skr::log::g_log_level) return;

    std::call_once(
        skr::log::g_start_once_flag,
        [] {
            skr::log::g_logger = skr::SPtr<skr::log::Logger>::Create();
            
            ::atexit(+[]() {
                auto worker = skr::log::LogWorkerSingleton::_weak_this.lock();
                SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
            });
        }
    );

    va_list va_args;
    va_start(va_args, fmt);
    skr::log::g_logger->log(kLogLevel, (const char8_t*)fmt, va_args);
    va_end(va_args);
}

RUNTIME_EXTERN_C 
void log_finalize()
{
    {
        if (auto worker = skr::log::LogWorkerSingleton::_weak_this.lock())
            worker->drain();
    }
    skr::log::g_logger.reset();

    auto worker = skr::log::LogWorkerSingleton::_weak_this.lock();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}