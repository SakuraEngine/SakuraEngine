#include "SkrCore/log.h"
#include "SkrCore/log/logger.hpp"
#include "SkrCore/async/wait_timeout.hpp"
#include "./log_manager.hpp"

#include "SkrProfile/profile.h"

namespace skr {
namespace logging {

ThreadToken::ThreadToken(LogQueue& q) SKR_NOEXCEPT
    : ptok_(q.queue_), backtraces_(4)
{

}

int64_t ThreadToken::query_cnt() const SKR_NOEXCEPT
{
    return skr_atomic_load_relaxed(&tls_cnt_);
}

EFlushStatus ThreadToken::query_status() const SKR_NOEXCEPT
{
    return static_cast<EFlushStatus>(skr_atomic_load_relaxed(&flush_status_));
}

LogElement::LogElement(LogEvent ev, ThreadToken* ptok) SKR_NOEXCEPT
    : event(ev), tok(ptok), valid(true)
{

}

LogElement::LogElement() SKR_NOEXCEPT
    : event(nullptr, LogLevel::kTrace, {}), valid(false)
{
    
}

LogQueue::LogQueue() SKR_NOEXCEPT
    : ctok_(queue_)
{
    skr_init_rw_mutex(&tids_mutex_);
}

LogQueue::~LogQueue() SKR_NOEXCEPT
{
    skr_destroy_rw_mutex(&tids_mutex_);
}

void LogQueue::finish(const LogElement& e) SKR_NOEXCEPT
{
    skr_atomic_fetch_add_relaxed(&total_cnt_, -1);
    auto last = skr_atomic_fetch_add_relaxed(&e.tok->tls_cnt_, -1);
    if (last == 1)
    {
        int32_t expected = kFlushing;
        skr_atomic_compare_exchange_strong(&e.tok->flush_status_, &expected, (int32_t)kFlushed);
    }
}

void LogQueue::push(LogEvent ev, const skr::String&& what, bool backtrace) SKR_NOEXCEPT
{
    auto ptok_ = on_push(ev, backtrace);
    SKR_ASSERT(ptok_);
    
    auto element = LogElement(ev, ptok_);

    element.format = std::move(what);
    element.need_format = false;
    
    if (!backtrace)
        queue_.enqueue(ptok_->ptok_, std::move(element));
    else
        ptok_->backtraces_.add(element);
}

void LogQueue::push(LogEvent ev, const skr::StringView format, ArgsList&& args, bool backtrace) SKR_NOEXCEPT
{
    auto ptok_ = on_push(ev, backtrace);
    SKR_ASSERT(ptok_);
    auto element = LogElement(ev, ptok_);

    element.format = format;
    element.args = std::move(args);
    element.need_format = true;

    if (!backtrace)
        queue_.enqueue(ptok_->ptok_, std::move(element));
    else
        ptok_->backtraces_.add(element);
}

void LogQueue::mark_flushing(SThreadID tid) SKR_NOEXCEPT
{
    if (auto tok = query_token(tid))
    {
        int32_t expected = kNoFlush;
        if (tok->query_cnt() != 0) // if there is no log in this thread, we don't need to flush
            skr_atomic_compare_exchange_strong(&tok->flush_status_, &expected, (int32_t)kFlushing);
        else
            skr_atomic_compare_exchange_strong(&tok->flush_status_, &expected, (int32_t)kFlushed);
    }
}

bool LogQueue::try_dequeue(LogElement& element) SKR_NOEXCEPT
{
    if (queue_.try_dequeue(ctok_, element))
    {
        return true;
    }
    return false;
}

bool LogQueue::try_dequeue_from(ThreadToken* tok, LogElement& element) SKR_NOEXCEPT
{
    if (queue_.try_dequeue_from_producer(tok->ptok_, element))
    {
        return true;
    }
    return false;
}

ThreadToken* LogQueue::query_token(SThreadID tid) const SKR_NOEXCEPT
{
    auto iter = thread_id_map_.find(tid);
    if (iter != thread_id_map_.end())
        return iter->second.get();
    return 0;
}

ThreadToken* LogQueue::query_flushing() const SKR_NOEXCEPT
{
    skr_rw_mutex_acquire_r(&tids_mutex_);
    tids_cpy_ = tids_;
    skr_rw_mutex_release_r(&tids_mutex_);

    for (uint64_t tid : tids_cpy_)
    {
        auto iter = thread_id_map_.find(tid);
        if (iter != thread_id_map_.end())
        {
            if (skr_atomic_load_relaxed(&iter->second->flush_status_) == kFlushing)
            {
                return iter->second.get();
            }
        }
    }
    return nullptr;
}

int64_t LogQueue::query_cnt() const SKR_NOEXCEPT
{
    return skr_atomic_load_relaxed(&total_cnt_);
}

ThreadToken* LogQueue::on_push(const LogEvent& ev, bool backtrace) SKR_NOEXCEPT
{
    const auto tid = ev.get_thread_id();
    auto iter = thread_id_map_.find(tid);
    if (iter == thread_id_map_.end())
    {
        skr_rw_mutex_acquire_w(&tids_mutex_);
        tids_.add(tid);
        skr_rw_mutex_release_w(&tids_mutex_);

        thread_id_map_.emplace(tid, skr::SP<ThreadToken>::New(*this));
    }
    
    if (auto token = thread_id_map_[tid].get())
    {
        if (backtrace) 
            return token;
        skr_atomic_fetch_add_relaxed(&total_cnt_, 1);
        skr_atomic_fetch_add_relaxed(&token->tls_cnt_, 1);
        return token;
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return nullptr;
}

LogWorker::LogWorker(const ServiceThreadDesc& desc) SKR_NOEXCEPT
    : AsyncService(desc), queue_(SP<LogQueue>::New())
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
    wait_timeout<u8"WaitLogWorkerStop">([&] { return get_status() == skr::ServiceThread::kStatusStopped; });
    exit();
}

void LogWorker::add_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers_.add(logger);
}

void LogWorker::remove_logger(Logger* logger) SKR_NOEXCEPT
{
    drain();
    loggers_.remove(logger);
}

bool LogWorker::predicate() SKR_NOEXCEPT
{
    return queue_->query_cnt();
}

void LogWorker::process_logs() SKR_NOEXCEPT
{
    LogManagerImpl::gLogManager->tscns_.calibrate();
    LogElement e;

    // try flush
    while (auto flush_tok = queue_->query_flushing())
    {
        SKR_ASSERT(flush_tok->query_status() == kFlushing);
        while (queue_->try_dequeue_from(flush_tok, e))
        {
            patternAndSink(e);
        }
        SKR_ASSERT(flush_tok->query_cnt() == 0);

        int32_t expected = kFlushing;
        skr_atomic_compare_exchange_strong(&flush_tok->flush_status_, &expected, (int32_t)kFlushed);
    }

    // normal polling    
    const uint32_t N = 16; // avoid too many logs in one cycle
    uint32_t n = 0;
    while (queue_->try_dequeue(e))
    {
        patternAndSink(e);

        n++;
        if (n >= N) break;
    }

    // flush sinks
    LogManagerImpl::gLogManager->FlushAllSinks();
}

void LogWorker::flush(SThreadID tid) SKR_NOEXCEPT
{
    queue_->mark_flushing(tid);
    this->awake();
    if (auto tok = queue_->query_token(tid))
    {
        while (skr_atomic_load_relaxed(&tok->flush_status_) != kFlushed)
        {
            if (this->t.get_id() == tid) // flush self
                process_logs();
            else
                skr_thread_sleep(0);
        }
        SKR_ASSERT(skr_atomic_load_relaxed(&tok->tls_cnt_) == 0);
        LogManagerImpl::gLogManager->FlushAllSinks();
        
        int32_t expected = kFlushed;
        skr_atomic_compare_exchange_strong(&tok->flush_status_, &expected, (int32_t)kNoFlush);
    }
    else
        SKR_UNREACHABLE_CODE();
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
        SkrZoneScopedNC("Dispatch", tracy::Color::Orchid3);
        process_logs();
    }
    return ASYNC_RESULT_OK;
}

void LogWorker::patternAndSink(const LogElement& e) SKR_NOEXCEPT
{
    if (e.valid)
    {
        SkrZoneScopedNC("LogSingle", tracy::Color::Orchid1);
        const auto& what = e.need_format ? 
            formatter_.format(e.format, e.args) :
            e.format;
        skr::logging::LogManagerImpl::gLogManager->PatternAndSink(e.event, what.view());
        
        if (auto should_backtrace = LogManagerImpl::gLogManager->ShouldBacktrace(e.event))
        {
            SkrZoneScopedNC("Backtrace", tracy::Color::Orchid2);
            const auto tid = (SThreadID)e.event.get_thread_id();
            if (auto token = queue_->query_token(tid))
            {
                for (int64_t i = token->backtraces_.get_size() - 1; i >= 0; i--)
                {
                    const auto bt_e = token->backtraces_.get(i);
                    if (bt_e.valid)
                    {
                        const auto& bt_what = bt_e.need_format ? 
                            formatter_.format(bt_e.format, bt_e.args) :
                            bt_e.format;
                        skr::logging::LogManagerImpl::gLogManager->PatternAndSink(bt_e.event, bt_what.view());
                    }
                }
                token->backtraces_.zero();
            }
        }

        queue_->finish(e);
    }
}

} } // namespace skr::logging

SKR_EXTERN_C 
void skr_log_flush()
{
    auto worker = skr::logging::LogManagerImpl::gLogManager->TryGetWorker();
    if (worker)
    {
        auto tid = skr_current_thread_id();
        worker->flush(tid);
    }    
}

SKR_EXTERN_C
void skr_log_initialize_async_worker()
{
    skr::logging::LogManagerImpl::gLogManager->InitializeAsyncWorker();

    auto worker = skr::logging::LogManagerImpl::gLogManager->TryGetWorker();
    SKR_ASSERT(worker && "worker must not be null & something is wrong with initialization!");

    ::atexit(+[]() {
        auto worker = skr::logging::LogManagerImpl::gLogManager->TryGetWorker();
        SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
    });
}
