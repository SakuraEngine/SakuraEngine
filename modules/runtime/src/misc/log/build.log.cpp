#include "platform/thread.h"
#include "log_rdtsc.hpp"
#include "log_queue.hpp"
#include "log_worker.hpp"
#include "misc/log/logger.hpp"

#include <thread>
#include <csignal>
#include <stdarg.h> // va_start
#include <stdio.h> // ::atexit

#include "tracy/Tracy.hpp"

namespace skr {
namespace log {

const char* kLogMemoryName = "sakura::log";

LogEvent::LogEvent(LogLevel level) SKR_NOEXCEPT
    : level(level), thread_id(skr_current_thread_id()), timestamp(skr::log::rdtsc())
{

}

LogFormatter::~LogFormatter() SKR_NOEXCEPT
{

}

skr::string const& LogFormatter::format(const skr::string& format, const ArgsList& args_list)
{
    args_list.format_(*this);
    return formatted_string;
}

LogPattern::~LogPattern() SKR_NOEXCEPT
{

}

skr::string const& LogPattern::pattern(const LogEvent& event, skr::string_view formatted_message)
{
    const auto ascii_time = event.timestamp;
    const auto level_id = (uint32_t)event.level;
    const auto thread_id = event.thread_id;
    const auto message = formatted_message;
    formatted_string = skr::format(calculated_format, 
        ascii_time, thread_id, 0, 
        level_id, u8"root", message
    );
    return formatted_string;
}

Logger::Logger() SKR_NOEXCEPT
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->add_logger(this);
    }
}

Logger::~Logger() SKR_NOEXCEPT
{
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->remove_logger(this);
    }
}

bool Logger::canPushToQueue() const SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
    return worker;
}

bool Logger::tryPushToQueue(LogEvent ev, skr::string_view format, ArgsList&& args_list) SKR_NOEXCEPT
{
    auto worker = LogManager::TryGetWorker();
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
    auto worker = LogManager::TryGetWorker();
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
    if (auto worker = LogManager::TryGetWorker())
    {
        worker->awake();
    }
}

LogElement::LogElement(LogEvent ev) SKR_NOEXCEPT
    : event(ev)
{

}

LogElement::LogElement() SKR_NOEXCEPT
    : event(LogLevel::kTrace)
{
    
}

SAtomic64 LogManager::available_ = 0;
eastl::unique_ptr<LogWorker> LogManager::worker_ = nullptr;
LogPatternMap LogManager::patterns_ = {};
using namespace skr::guid::literals;
const skr_guid_t LogConstants::kDefaultPatternId = u8"c236a30a-c91e-4b26-be7c-c7337adae428"_guid;

void LogManager::Initialize() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) != 0)
        return;

    // register default pattern
    patterns_.emplace(LogConstants::kDefaultPatternId, eastl::make_unique<LogPattern>());

    // start worker
    if (!worker_)
    {
        worker_ = eastl::make_unique<LogWorker>(kLoggerWorkerThreadDesc);
        worker_->run();
    }
    skr_atomic64_cas_relaxed(&available_, 0, 1);
}

void LogManager::Finalize() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) != 0)
    {
        worker_.reset();
        skr_atomic64_cas_relaxed(&available_, 1, 0);
    }
}

LogWorker* LogManager::TryGetWorker() SKR_NOEXCEPT
{
    if (skr_atomic64_load_acquire(&available_) == 0)
        return nullptr;
    return worker_.get();
}

skr_guid_t LogManager::RegisterPattern(eastl::unique_ptr<LogPattern> pattern)
{
    auto guid = skr_guid_t();
    skr_make_guid(&guid);
    patterns_.emplace(guid, skr::move(pattern));
    return guid;
}

LogPattern* LogManager::QueryPattern(skr_guid_t guid)
{
    auto it = patterns_.find(guid);
    if (it != patterns_.end())
        return it->second.get();
    return nullptr;
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
    LogElement e;
    while (queue_->try_dequeue(e))
    {
        ZoneScopedNC("LogSingle", tracy::Color::Orchid1);
        const auto& what = e.need_format ? 
            formatter_.format(e.format, e.args) :
            e.format;
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
    skr::log::LogManager::Initialize();

    auto worker = skr::log::LogManager::TryGetWorker();
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
                auto worker = skr::log::LogManager::TryGetWorker();
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
        if (auto worker = skr::log::LogManager::TryGetWorker())
            worker->drain();
    }
    skr::log::g_logger.reset();
    skr::log::LogManager::Finalize();

    auto worker = skr::log::LogManager::TryGetWorker();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}