#pragma once
#include "SkrContainersDef/hashmap.hpp"
#include "SkrBase/atomic/atomic.h"
#include "SkrCore/memory/sp.hpp"
#include "SkrOS/thread.h"
#include "SkrCore/log/log_base.hpp"
#include "SkrCore/log/log_formatter.hpp"

#include "SkrContainersDef/concurrent_queue.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/ring_buffer.hpp"

namespace skr
{
namespace logging
{

struct LogQueue;

enum EFlushStatus
{
    kNoFlush  = 0,
    kFlushing = 1,
    kFlushed  = 2
};

struct LogElement {
    LogElement() SKR_NOEXCEPT;

private:
    LogElement(LogEvent ev, struct ThreadToken* ptok) SKR_NOEXCEPT;

    friend struct LogQueue;
    friend struct LogWorker;

    LogEvent            event;
    struct ThreadToken* tok;
    skr::String         format;
    ArgsList            args;
    bool                need_format = true;
    bool                valid       = false;
};

struct ThreadToken {
    ThreadToken(LogQueue& q) SKR_NOEXCEPT;
    int64_t      query_cnt() const SKR_NOEXCEPT;
    EFlushStatus query_status() const SKR_NOEXCEPT;

protected:
    friend struct LogWorker;
    friend struct LogQueue;
    SAtomic64                                   tls_cnt_      = 0;
    SAtomic32                                   flush_status_ = kNoFlush;
    skr::ProducerToken                          ptok_;
    skr::SimpleThreadSafeRingBuffer<LogElement> backtraces_;
};

struct LogQueue {
public:
    LogQueue() SKR_NOEXCEPT;
    ~LogQueue() SKR_NOEXCEPT;

    void push(LogEvent ev, const skr::String&& what, bool backtrace) SKR_NOEXCEPT;
    void push(LogEvent ev, const skr::StringView format, ArgsList&& args, bool backtrace) SKR_NOEXCEPT;
    void mark_flushing(SThreadID tid) SKR_NOEXCEPT;

    bool try_dequeue(LogElement& element) SKR_NOEXCEPT;
    bool try_dequeue_from(ThreadToken* tok, LogElement& element) SKR_NOEXCEPT;
    void finish(const LogElement& element) SKR_NOEXCEPT;

    ThreadToken* query_token(SThreadID tid) const SKR_NOEXCEPT;
    ThreadToken* query_flushing() const SKR_NOEXCEPT;
    int64_t      query_cnt() const SKR_NOEXCEPT;

private:
    [[nodiscard]] ThreadToken* on_push(const LogEvent& ev, bool backtrace) SKR_NOEXCEPT;

    // formatter & args
    template <int BLK_SIZE = 256>
    struct LogQueueTraits : public ConcurrentQueueDefaultTraits {
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static const int  BLOCK_SIZE               = BLK_SIZE;

        static inline void* malloc(size_t size) SKR_NOEXCEPT { return sakura_mallocN(size, kLogMemoryName); }
        static inline void  free(void* ptr) SKR_NOEXCEPT { return sakura_freeN(ptr, kLogMemoryName); }
    };

    friend struct ThreadToken;
    SAtomic64 total_cnt_ = 0;
    // MPSC
    skr::ConsumerToken                                       ctok_;
    skr::ParallelFlatHashMap<uint64_t, skr::SP<ThreadToken>> thread_id_map_;
    skr::ConcurrentQueue<LogElement, LogQueueTraits<256>>    queue_;

    skr::Vector<uint64_t>         tids_;
    mutable SRWMutex              tids_mutex_;
    mutable skr::Vector<uint64_t> tids_cpy_;
};

} // namespace logging
} // namespace skr