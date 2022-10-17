#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"

namespace skr
{
struct RUNTIME_API SRCBlock
{
    SRCBlock() = default;
    static SRCBlock* Create() SKR_NOEXCEPT; 
    static void Destroy( SRCBlock* b) SKR_NOEXCEPT;

    void add_refcount() SKR_NOEXCEPT;
    void add_weak_refcount() SKR_NOEXCEPT;
    void weak_release() SKR_NOEXCEPT;
    void release(uint32_t* pRC = nullptr) SKR_NOEXCEPT;

    SRCBlock* lock() SKR_NOEXCEPT;

    SAtomic32 refcount = 1;
    SAtomic32 weak_refcount = 1;
};

template <bool Use> struct SRCInst { };
template <> struct SRCInst<true> 
{
    RUNTIME_API void allocate_block();
    inline uint32_t use_count() const { return block ? block->refcount : 0; }
    inline bool unique() const { return block ? (block->weak_refcount == 1) : false; }
    inline bool equivalent_rc_ownership(const SRCInst<true>& lp) const
    {
        return block == lp.block;
    }
protected:
    inline void SwapRCBlock(SRCInst<true>& other)
    {
        auto* pBlock = other.block;
        other.block = block;
        block = pBlock;
    }
    inline void CopyRCBlockFrom(const SRCInst<true>& other)
    {
        block = other.block;
    }
    inline void MoveRCBlockFrom(SRCInst<true>& other)
    {
        block = other.block;
        other.block = nullptr;
    }
    SRCBlock* block = nullptr; 
};
} // namespace skr