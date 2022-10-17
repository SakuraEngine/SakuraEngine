#include <new>
#include "containers/detail/shared_rc.hpp"
#include "platform/memory.h"

skr::SRCBlock* skr::SRCBlock::Create() SKR_NOEXCEPT { return SkrNew<SRCBlock>(); }
void skr::SRCBlock::Destroy(SRCBlock* b) SKR_NOEXCEPT { SkrDelete(b); }

void skr::SRCBlock::add_refcount() SKR_NOEXCEPT
{
    skr_atomic32_add_relaxed(&refcount, 1);
    skr_atomic32_add_relaxed(&weak_refcount, 1);
}

void skr::SRCBlock::add_weak_refcount() SKR_NOEXCEPT
{
    skr_atomic32_add_relaxed(&weak_refcount, 1);
}

void skr::SRCBlock::weak_release() SKR_NOEXCEPT
{
    auto weak_prev = skr_atomic32_add_relaxed(&weak_refcount, -1);
    if (weak_prev <= 1)
    {
        SRCBlock::Destroy(this);
    }
}

void skr::SRCBlock::release(uint32_t* pRC) SKR_NOEXCEPT
{
    SKR_ASSERT((refcount > 0) && (weak_refcount > 0));
    auto prev = skr_atomic32_add_relaxed(&refcount, -1);
    if (pRC) *pRC = prev - 1;
    if (prev > 1)
    {
        skr_atomic32_add_relaxed(&weak_refcount, -1);
    }
    else
    {
        auto weak_prev = skr_atomic32_add_relaxed(&weak_refcount, -1);
        if (weak_prev <= 1)
        {
            SRCBlock::Destroy(this);
        }
    }
}

skr::SRCBlock* skr::SRCBlock::lock() SKR_NOEXCEPT
{
    for (int32_t refCountTemp = refcount; refCountTemp != 0; refCountTemp = refcount)
    {
        if (bool cas = skr_atomic32_cas_relaxed(&refcount, refCountTemp, refCountTemp + 1); cas)
        {
            skr_atomic32_add_relaxed(&weak_refcount, 1);
            return this;
        }
    }
    return nullptr;
}

void skr::SRCInst<true>::allocate_block()
{
    block = SRCBlock::Create();
} 