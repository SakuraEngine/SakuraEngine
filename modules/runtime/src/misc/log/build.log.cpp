#include "platform/thread.h"
#include "misc/log.h"
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
    : level(level), timestamp(skr_sys_get_time()), thread_id(skr_current_thread_id())
{

}

LogFormatter::~LogFormatter() SKR_NOEXCEPT
{

}

skr::string const& LogFormatter::format(const skr::string& format, const ArgsList& args_list)
{
    args_list.format_(format, *this);
    return formatted_string;
}

LogPattern::~LogPattern() SKR_NOEXCEPT
{

}

skr::string const& LogPattern::pattern(const LogEvent& event, skr::string_view formatted_message)
{
    const auto ascii_time = event.timestamp;
    const auto level_id = (uint32_t)event.level;
    const auto level_name = LogConstants::kLogLevelNameLUT[level_id];
    const auto thread_id = event.thread_id;
    const auto message = formatted_message;
    formatted_string = skr::format(calculated_format, 
        ascii_time, thread_id, 0, 
        level_name, u8"root", message
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

Logger* Logger::GetDefault() SKR_NOEXCEPT
{
    return LogManager::GetDefaultLogger();
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

using namespace skr::guid::literals;
const skr_guid_t LogConstants::kDefaultPatternId = u8"c236a30a-c91e-4b26-be7c-c7337adae428"_guid;
skr::log::LogLevel LogConstants::gLogLevel = skr::log::LogLevel::kTrace;

SAtomic64 LogManager::available_ = 0;
eastl::unique_ptr<LogWorker> LogManager::worker_ = nullptr;
LogPatternMap LogManager::patterns_ = {};
std::once_flag g_start_once_flag;
eastl::unique_ptr<skr::log::Logger> LogManager::logger_ = nullptr;

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
    // skr::log::LogManager::logger_.reset();
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

Logger* LogManager::GetDefaultLogger() SKR_NOEXCEPT
{
    std::call_once(
        skr::log::g_start_once_flag,
        [] {
            skr::log::LogManager::logger_ = eastl::make_unique<skr::log::Logger>();
        }
    );
    return logger_.get();
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
            
        auto pattern = LogManager::QueryPattern(LogConstants::kDefaultPatternId);
        const auto& output = pattern->pattern(e.event, what.view());

        printf("%s\n", output.c_str());
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

RUNTIME_EXTERN_C
void log_set_level(int level)
{
    const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];\
    skr::log::LogConstants::gLogLevel = kLogLevel;
}

RUNTIME_EXTERN_C 
void log_log(int level, const char* file, int line, const char* fmt, ...)
{
    const auto kLogLevel = skr::log::LogConstants::kLogLevelsLUT[level];
    if (kLogLevel < skr::log::LogConstants::gLogLevel) return;

    const auto Event = skr::log::LogEvent(kLogLevel);
    auto logger = skr::log::LogManager::GetDefaultLogger();

    va_list va_args;
    va_start(va_args, fmt);
    logger->log(Event, (const char8_t*)fmt, va_args);
    va_end(va_args);
}

RUNTIME_EXTERN_C 
void log_finalize()
{
    {
        if (auto worker = skr::log::LogManager::TryGetWorker())
            worker->drain();
    }
    skr::log::LogManager::Finalize();

    auto worker = skr::log::LogManager::TryGetWorker();
    SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
}