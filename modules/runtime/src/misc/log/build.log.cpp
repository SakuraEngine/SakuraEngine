#include "platform/thread.h"
#include "log_rdtsc.hpp"
#include "log_queue.hpp"
#include "log_worker.hpp"
#include "misc/log/logger.hpp"

#include <thread>
#include <csignal>
#include <stdio.h> // ::atexit

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {

const char* kLogMemoryName = "sakura::log";

LogEvent::LogEvent(LogLevel level) SKR_NOEXCEPT
    : level(level), thread_id(skr_current_thread_id()), timestamp(skr::log::rdtsc())
{

}

Logger::Logger() SKR_NOEXCEPT
{
    if (auto worker = LogWorkerSingleton::TryGet())
    {
        worker->add_logger(this);
    }
}

Logger::~Logger() SKR_NOEXCEPT
{
    if (auto worker = LogWorkerSingleton::TryGet())
    {
        worker->remove_logger(this);
    }
}

bool Logger::canPushToQueue() const SKR_NOEXCEPT
{
    auto worker = LogWorkerSingleton::TryGet();
    return worker;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string_view format, ArgsList<>&& args_list) SKR_NOEXCEPT
{
    auto worker = LogWorkerSingleton::TryGet();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, format, skr::move(args_list));
        notifyWorker();
        return true;
    }
    return false;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string&& what) SKR_NOEXCEPT
{
    auto worker = LogWorkerSingleton::TryGet();
    if (worker)
    {
        auto queue_ = worker->queue_;
        queue_->push(ev, skr::move(what));
        notifyWorker();
        return true;
    }
    return false;
}

void Logger::notifyWorker() SKR_NOEXCEPT
{
    if (auto worker = LogWorkerSingleton::TryGet())
    {
        worker->awake();
    }
}

LogQueueElement::LogQueueElement(LogEvent ev) SKR_NOEXCEPT
    : event(ev)
{

}

LogQueueElement::LogQueueElement() SKR_NOEXCEPT
    : event(LogLevel::kTrace)
{
    
}

skr::string_view LogQueueElement::produce() SKR_NOEXCEPT
{
    if (need_format)
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        // format = ...
    }
    return format.view();
}

eastl::unique_ptr<LogWorker> LogWorkerSingleton::_this = nullptr;
SAtomic64 LogWorkerSingleton::_available = 0;

void LogWorkerSingleton::Initialize() SKR_NOEXCEPT
{
    if (!_this)
    {
        _this = eastl::make_unique<LogWorker>(kLoggerWorkerThreadDesc);
        _this->run();
        skr_atomic64_cas_relaxed(&_available, 0, 1);
    }
}

LogWorker* LogWorkerSingleton::TryGet() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&_available) == 0)
        return nullptr;
    return _this.get();
}

void LogWorkerSingleton::Finalize() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&_available) != 0)
    {
        _this.reset();
        skr_atomic64_cas_relaxed(&_available, 1, 0);
    }
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
void log_initialize_async_worker()
{
    skr::log::LogWorkerSingleton::Initialize();

    auto worker = skr::log::LogWorkerSingleton::TryGet();
    SKR_ASSERT(worker && "worker must not be null & something is wrong with initialization!");
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

    const auto Event = skr::log::LogEvent(kLogLevel);
    std::call_once(
        skr::log::g_start_once_flag,
        [] {
            skr::log::g_logger = skr::SPtr<skr::log::Logger>::Create();
            
            ::atexit(+[]() {
                auto worker = skr::log::LogWorkerSingleton::TryGet();
                SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
            });
        }
    );

    va_list va_args;
    va_start(va_args, fmt);
    skr::log::g_logger->log(Event, (const char8_t*)fmt, va_args);
    va_end(va_args);
}

RUNTIME_EXTERN_C 
void log_finalize()
{
    {
        if (auto worker = skr::log::LogWorkerSingleton::TryGet())
            worker->drain();
    }
    skr::log::g_logger.reset();
    skr::log::LogWorkerSingleton::Finalize();

    auto worker = skr::log::LogWorkerSingleton::TryGet();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}