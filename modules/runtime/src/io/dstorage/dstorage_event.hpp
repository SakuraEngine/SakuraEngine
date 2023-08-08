#pragma once
#include "SkrRT/platform/dstorage.h"
#include "../common/io_request.hpp"
#include <EASTL/fixed_vector.h>

namespace skr {
namespace io {

struct SKR_RUNTIME_API DStorageEvent : public skr::SInterface
{
    IO_RC_OBJECT_BODY
public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<DStorageEvent*>(ptr);
            p->pool->deallocate(p); 
        };
    }

    bool okay() { return skr_dstorage_event_test(event); }

    friend struct SmartPool<DStorageEvent, DStorageEvent>;
protected:
    DStorageEvent(ISmartPoolPtr<DStorageEvent> pool, SkrDStorageQueueId queue) 
        : queue(queue), pool(pool)
    {
        if (!event)
            event = skr_dstorage_queue_create_event(queue);
    }
    ~DStorageEvent() SKR_NOEXCEPT 
    {
        if (event)
        {
            skr_dstorage_queue_free_event(queue, event);
        }
        queue = nullptr;
    }
    friend struct DStorageRAMReader;
    friend struct DStorageVRAMReader;
    eastl::fixed_vector<IOBatchId, 32> batches;
    SkrDStorageQueueId queue = nullptr;
    ISmartPoolPtr<DStorageEvent> pool = nullptr;
    SkrDStorageEventId event = nullptr;
};

} // namespace io
} // namespace skr