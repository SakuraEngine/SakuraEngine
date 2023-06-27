#pragma once
#include "args.hpp"
#include "platform/thread.h"
#include "containers/string.hpp"
#include "containers/concurrent_queue.h"

namespace skr {
namespace log {

extern const char* kLogMemoryName;

enum class LogLevel : uint32_t
{
    kTrace,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kFatal,
    kCount
};

struct LogEvent
{
    LogEvent(LogLevel level) SKR_NOEXCEPT
        : level(level)
    {
        thread_id = skr_current_thread_id();

    }
    LogLevel level;
    SThreadID thread_id;
    bool flush = false;
};

struct LogQueueElement
{
    skr::string_view produce()
    {
        if (need_format)
        {
            SKR_UNIMPLEMENTED_FUNCTION();
            // format = ...
        }
        return format.view();
    }

private:
    friend struct LogQueue;
    friend struct LogWorker;
    LogQueueElement(LogEvent ev) SKR_NOEXCEPT
        : event(ev)
    {
    }
    LogQueueElement() SKR_NOEXCEPT
        : event(LogLevel::kTrace)
    {
    }
    LogEvent event;
    skr::string format;
    ArgsList<> args;
    bool need_format = true;
};

struct LogQueue
{
public:
    void push(LogEvent ev, const skr::string&& what) SKR_NOEXCEPT
    {
        auto element = LogQueueElement(ev);

        element.format = skr::move(what);
        element.need_format = false;
        
        queue_.enqueue(skr::move(element));
        skr_atomic64_add_relaxed(&cnt_, 1);
    }
    
    template <typename...Args>
    void push(LogEvent ev, const skr::string_view format, Args&&...args) SKR_NOEXCEPT
    {
        auto element = LogQueueElement(ev);

        element.format = format.raw();
        element.args.push(std::forward<Args>(args)...);
        element.need_format = true;

        queue_.enqueue(skr::move(element));
        skr_atomic64_add_relaxed(&cnt_, 1);
    }

    bool try_dequeue(LogQueueElement& element)
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
    skr::ConcurrentQueue<LogQueueElement, LogQueueTraits> queue_;
    SAtomic64 cnt_ = 0;
};

} } // namespace skr::log