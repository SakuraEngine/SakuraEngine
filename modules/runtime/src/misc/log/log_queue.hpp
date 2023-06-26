#pragma once
#include "args.hpp"
#include "platform/thread.h"
#include "containers/string.hpp"
#include "containers/concurrent_queue.h"

namespace skr {
namespace log {

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
    }
    
    template <typename...Args>
    void push(LogEvent ev, const skr::string_view format, Args&&...args) SKR_NOEXCEPT
    {
        auto element = LogQueueElement(ev);

        element.format = format.raw();
        element.args.push(std::forward<Args>(args)...);
        element.need_format = true;

        queue_.enqueue(skr::move(element));
    }

    bool try_dequeue(LogQueueElement& element)
    {
        return queue_.try_dequeue(element);
    }
    
private:
    // formatter & args
    struct LogQueueTraits : public ConcurrentQueueDefaultTraits
    {
        static const int BLOCK_SIZE = 256;
    };
    skr::ConcurrentQueue<LogQueueElement, LogQueueTraits> queue_;
};


} } // namespace skr::log