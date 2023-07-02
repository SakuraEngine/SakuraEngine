#pragma once
#include "args.hpp"
#include "containers/string.hpp"
#include "containers/concurrent_queue.h"

namespace skr {
namespace log {

struct LogQueueElement
{
    skr::string format;
    ArgsList<> args;
};

struct LogQueue
{
public:
    template <typename...Args>
    void push(const skr::string_view format, Args&&...args)
    {
        auto element = LogQueueElement();
        element.format = format.raw();
        element.args.push(std::forward<Args>(args)...);
        queue_.enqueue(element);
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