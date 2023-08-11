#include "SkrRT/misc/log.h"
#include "SkrRT/misc/log/logger.hpp"
#include "misc/log/log_manager.hpp"

#include "SkrProfile/profile.h"

namespace skr {
namespace log {

ThreadToken::ThreadToken(LogQueue& q) SKR_NOEXCEPT
    : ptok_(q.queue_), backtraces_(4)
{

}

int64_t ThreadToken::query_cnt() const SKR_NOEXCEPT
{
    return skr_atomic64_load_relaxed(&tls_cnt_);
}

EFlushStatus ThreadToken::query_status() const SKR_NOEXCEPT
{
    return static_cast<EFlushStatus>(skr_atomic32_load_relaxed(&flush_status_));
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
    skr_atomic64_add_relaxed(&total_cnt_, -1);
    auto last = skr_atomic64_add_relaxed(&e.tok->tls_cnt_, -1);
    if (last == 1)
    {
        skr_atomic32_cas_relaxed(&e.tok->flush_status_, kFlushing, kFlushed);
    }
}

void LogQueue::push(LogEvent ev, const skr::string&& what, bool backtrace) SKR_NOEXCEPT
{
    auto ptok_ = on_push(ev, backtrace);
    SKR_ASSERT(ptok_);
    
    auto element = LogElement(ev, ptok_);

    element.format = skr::move(what);
    element.need_format = false;
    
    if (!backtrace)
        queue_.enqueue(ptok_->ptok_, skr::move(element));
    else
        ptok_->backtraces_.add(element);
}

void LogQueue::push(LogEvent ev, const skr::string_view format, ArgsList&& args, bool backtrace) SKR_NOEXCEPT
{
    auto ptok_ = on_push(ev, backtrace);
    SKR_ASSERT(ptok_);
    auto element = LogElement(ev, ptok_);

    element.format = format.raw();
    element.args = skr::move(args);
    element.need_format = true;

    if (!backtrace)
        queue_.enqueue(ptok_->ptok_, skr::move(element));
    else
        ptok_->backtraces_.add(element);
}

void LogQueue::mark_flushing(SThreadID tid) SKR_NOEXCEPT
{
    if (auto tok = query_token(tid))
    {
        if (tok->query_cnt() != 0) // if there is no log in this thread, we don't need to flush
            skr_atomic32_cas_relaxed(&tok->flush_status_, kNoFlush, kFlushing);
        else
            skr_atomic32_cas_relaxed(&tok->flush_status_, kNoFlush, kFlushed);
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
            if (skr_atomic32_load_relaxed(&iter->second->flush_status_) == kFlushing)
            {
                return iter->second.get();
            }
        }
    }
    return nullptr;
}

int64_t LogQueue::query_cnt() const SKR_NOEXCEPT
{
    return skr_atomic64_load_relaxed(&total_cnt_);
}

ThreadToken* LogQueue::on_push(const LogEvent& ev, bool backtrace) SKR_NOEXCEPT
{
    const auto tid = ev.get_thread_id();
    auto iter = thread_id_map_.find(tid);
    if (iter == thread_id_map_.end())
    {
        skr_rw_mutex_acquire_w(&tids_mutex_);
        tids_.emplace_back(tid);
        skr_rw_mutex_release_w(&tids_mutex_);

        thread_id_map_.emplace(tid, eastl::make_unique<ThreadToken>(*this));
    }
    
    if (auto token = thread_id_map_[tid].get())
    {
        if (backtrace) 
            return token;
        skr_atomic64_add_relaxed(&total_cnt_, 1);
        skr_atomic64_add_relaxed(&token->tls_cnt_, 1);
        return token;
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
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
    LogManager::Get()->tscns_.calibrate();
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
        skr_atomic32_cas_relaxed(&flush_tok->flush_status_, kFlushing, kFlushed);
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
    LogManager::Get()->FlushAllSinks();
}

void LogWorker::flush(SThreadID tid) SKR_NOEXCEPT
{
    SKR_ASSERT(this->t.get_id() != tid && "flush from worker thread is not allowed");
    queue_->mark_flushing(tid);
    this->awake();
    if (auto tok = queue_->query_token(tid))
    {
        while (skr_atomic32_load_relaxed(&tok->flush_status_) != kFlushed)
        {
            skr_thread_sleep(0);
        }
        SKR_ASSERT(skr_atomic64_load_relaxed(&tok->tls_cnt_) == 0);
        LogManager::Get()->FlushAllSinks();
        skr_atomic32_cas_relaxed(&tok->flush_status_, kFlushed, kNoFlush);
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
        skr::log::LogManager::Get()->PatternAndSink(e.event, what.view());
        
        if (auto should_backtrace = LogManager::Get()->ShouldBacktrace(e.event))
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
                        skr::log::LogManager::Get()->PatternAndSink(bt_e.event, bt_what.view());
                    }
                }
                token->backtraces_.zero();
            }
        }

        queue_->finish(e);
    }
}

} } // namespace skr::log

SKR_EXTERN_C 
void skr_log_flush()
{
    auto worker = skr::log::LogManager::Get()->TryGetWorker();
    if (worker)
    {
        auto tid = skr_current_thread_id();
        worker->flush(tid);
    }    
}

SKR_EXTERN_C
void skr_log_initialize_async_worker()
{
    skr::log::LogManager::Get()->InitializeAsyncWorker();

    auto worker = skr::log::LogManager::Get()->TryGetWorker();
    SKR_ASSERT(worker && "worker must not be null & something is wrong with initialization!");

    ::atexit(+[]() {
        auto worker = skr::log::LogManager::Get()->TryGetWorker();
        SKR_ASSERT(!worker && "worker must be null & properly stopped at exit!");
    });
}
