#pragma once
#include "SkrRT/io/io.h"
#include "pool.hpp"
#include "SkrOS/thread.h"
#include "SkrBase/misc/defer.hpp"
#include "SkrContainers/vector.hpp"

namespace skr
{
namespace io
{

struct IOBatchBase : public IIOBatch {
    SKR_RC_IMPL(override)
public:
    void reserve(uint64_t n) SKR_NOEXCEPT override
    {
        requests.reserve(n);
    }

    skr::span<IORequestId> get_requests() SKR_NOEXCEPT override
    {
        skr_rw_mutex_acquire_r(&rw_lock);
        SKR_DEFER({ skr_rw_mutex_release_r(&rw_lock); });
        return { requests.data(), requests.size() };
    }

    void                    set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT override { priority = pri; }
    SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT override { return priority; }

    const bool can_use_dstorage = true; // TODO: make it configurable

protected:
    friend struct RunnerBase;
    void addRequest(IORequestId rq) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&rw_lock);
        SKR_DEFER({ skr_rw_mutex_release_w(&rw_lock); });
        requests.add(rq);
    }

    void removeCancelledRequest(IORequestId rq) SKR_NOEXCEPT
    {
        skr_rw_mutex_acquire_w(&rw_lock);
        SKR_DEFER({ skr_rw_mutex_release_w(&rw_lock); });
        requests.remove_all_if([rq](IORequestId r) { return r == rq; });
    }

private:
    SkrAsyncServicePriority  priority;
    SRWMutex                 rw_lock;
    skr::Vector<IORequestId> requests;

public:
    void skr_rc_delete() override
    {
        pool->deallocate(this);
    }
    friend struct SmartPool<IOBatchBase, IIOBatch>;

protected:
    IOBatchBase(ISmartPoolPtr<IIOBatch> pool, IIOService* service, const uint64_t sequence) SKR_NOEXCEPT
        : sequence(sequence),
          pool(pool),
          service(service)
    {
        skr_init_rw_mutex(&rw_lock);
    }

    ~IOBatchBase() SKR_NOEXCEPT
    {
        skr_destroy_rw_mutex(&rw_lock);
    }

    const uint64_t          sequence;
    ISmartPoolPtr<IIOBatch> pool    = nullptr;
    IIOService*             service = nullptr;
};

using BatchPtr     = skr::RC<IIOBatch>;
using IOBatchQueue = IOConcurrentQueue<BatchPtr>;
using IOBatchArray = skr::Vector<BatchPtr>;

} // namespace io
} // namespace skr