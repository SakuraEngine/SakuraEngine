#pragma once
#include "platform/atomic.h"
#include "misc/log/log_base.hpp"
#include "misc/log/log_formatter.hpp"

#include "containers/concurrent_queue.h"

namespace skr {
namespace log {

struct LogElement
{
private:
    LogElement(LogEvent ev) SKR_NOEXCEPT;
    LogElement() SKR_NOEXCEPT;
    
    friend struct LogQueue;
    friend struct LogWorker;

    LogEvent event;
    skr::string format;
    ArgsList args;
    bool need_format = true;
};

struct LogQueue
{
public:
    void push(LogEvent ev, const skr::string&& what) SKR_NOEXCEPT
    {
        auto element = LogElement(ev);

        element.format = skr::move(what);
        element.need_format = false;
        
        queue_.enqueue(skr::move(element));
        skr_atomic64_add_relaxed(&cnt_, 1);
    }
    
    void push(LogEvent ev, const skr::string_view format, ArgsList&& args) SKR_NOEXCEPT
    {
        auto element = LogElement(ev);

        element.format = format.raw();
        element.args = skr::move(args);
        element.need_format = true;

        queue_.enqueue(skr::move(element));
        skr_atomic64_add_relaxed(&cnt_, 1);
    }

    bool try_dequeue(LogElement& element)
    {
        if (queue_.try_dequeue(element))
        {
            skr_atomic64_add_relaxed(&cnt_, -1);
            return true;
        }
        return false;
    }
    
    int64_t query_cnt() const SKR_NOEXCEPT
    {
        return skr_atomic64_load_relaxed(&cnt_);
    }

private:
    // formatter & args
    struct LogQueueTraits : public ConcurrentQueueDefaultTraits
    {
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static const int BLOCK_SIZE = 256;

        static inline void* malloc(size_t size) { return sakura_mallocN(size, kLogMemoryName); }
        static inline void free(void* ptr) { return sakura_freeN(ptr, kLogMemoryName); }
    };
    skr::ConcurrentQueue<LogElement, LogQueueTraits> queue_;
    SAtomic64 cnt_ = 0;
};

} } // namespace skr::log