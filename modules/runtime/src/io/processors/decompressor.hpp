#pragma once
#include "io/io.h"
#include "../common/io_request.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {

template<typename I = IIORequestProcessor>
struct TaskDecompressorBase : public IIODecompressor<I>
{
    IO_RC_OBJECT_BODY
public:
    TaskDecompressorBase() SKR_NOEXCEPT 
    {
        for (uint32_t i = 0 ; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT ; ++i)
        {
            skr_atomic64_store_relaxed(&pending_counts[i], 0);
            skr_atomic64_store_relaxed(&processed_counts[i], 0);
        }
    }
    virtual ~TaskDecompressorBase() SKR_NOEXCEPT {}

    uint64_t processing_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomic64_load_acquire(&pending_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
            {
                count += skr_atomic64_load_acquire(&pending_counts[i]);
            }
            return count;
        }
    }

    uint64_t processed_count(SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_COUNT) const SKR_NOEXCEPT
    {
        if (priority != SKR_ASYNC_SERVICE_PRIORITY_COUNT)
        {
            return skr_atomic64_load_acquire(&processed_counts[priority]);
        }
        else
        {
            uint64_t count = 0;
            for (int i = 0; i < SKR_ASYNC_SERVICE_PRIORITY_COUNT; ++i)
            {
                count += skr_atomic64_load_acquire(&processed_counts[i]);
            }
            return count;
        }
    }

protected:
    SAtomic64 pending_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
    SAtomic64 processed_counts[SKR_ASYNC_SERVICE_PRIORITY_COUNT];
};

} // namespace io
} // namespace skr