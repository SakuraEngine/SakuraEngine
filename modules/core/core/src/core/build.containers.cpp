#include "SkrBase/atomic/atomic.h"
#include "SkrContainers/detail/shared_rc.hpp"
#include <new>

namespace skr
{
    SKR_CORE_API const char* kConcurrentQueueMemoryName = "ConcurrentQueues";
    SKR_CORE_API const char8_t* kOpenStringMemory = u8"OpenString";
}

void skr::SRCBlock::add_refcount() SKR_NOEXCEPT
{
    atomic_fetch_add_relaxed(&refcount, 1);
    atomic_fetch_add_relaxed(&weak_refcount, 1);
}

void skr::SRCBlock::add_weak_refcount() SKR_NOEXCEPT
{
    atomic_fetch_add_relaxed(&weak_refcount, 1);
}

void skr::SRCBlock::weak_release() SKR_NOEXCEPT
{
    auto weak_prev = atomic_fetch_add_relaxed(&weak_refcount, -1);
    if (weak_prev <= 1)
    {
        SRCBlock::Destroy(this);
    }
}

void skr::SRCBlock::release(uint32_t* pRC) SKR_NOEXCEPT
{
    SKR_ASSERT((refcount > 0) && (weak_refcount > 0));
    auto prev = atomic_fetch_add_relaxed(&refcount, -1);
    if (pRC) *pRC = prev - 1;
    if (prev > 1)
    {
        atomic_fetch_add_relaxed(&weak_refcount, -1);
    }
    else
    {
        this->free_value();
        auto weak_prev = atomic_fetch_add_relaxed(&weak_refcount, -1);
        if (weak_prev <= 1)
        {
            SRCBlock::Destroy(this);
        }
    }
}

skr::SRCBlock* skr::SRCBlock::lock() SKR_NOEXCEPT
{
    for (auto refCountTemp = atomic_load(&refcount); refCountTemp != 0; refCountTemp = refcount)
    {
        if (bool cas = atomic_compare_exchange_strong(&refcount, &refCountTemp, refCountTemp + 1); cas)
        {
            atomic_fetch_add_relaxed(&weak_refcount, 1);
            return this;
        }
    }
    return nullptr;
}