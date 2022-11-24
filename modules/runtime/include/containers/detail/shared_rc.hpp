#pragma once
#include "platform/configure.h"
#include "platform/atomic.h"
#include "platform/memory.h"

namespace skr
{
template<typename T>
struct DefaultDeleter
{
    SKR_CONSTEXPR DefaultDeleter() SKR_NOEXCEPT = default;

    template <typename U>  // Enable if T* can be constructed with U* (i.e. U* is convertible to T*).
    DefaultDeleter(const DefaultDeleter<U>&, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT {}

    void operator()(T* p) const SKR_NOEXCEPT { SkrDelete(p); }
};

struct RUNTIME_API SRCBlock
{
    SRCBlock() = default;
    virtual ~SRCBlock() = default;
    template<typename T, typename Deleter>
    static SRCBlock* Create(T* ptr, const Deleter& deleter) SKR_NOEXCEPT;
    static void Destroy(SRCBlock* b) SKR_NOEXCEPT;

    void add_refcount() SKR_NOEXCEPT;
    void add_weak_refcount() SKR_NOEXCEPT;
    void weak_release() SKR_NOEXCEPT;
    void release(uint32_t* pRC = nullptr) SKR_NOEXCEPT;

    SRCBlock* lock() SKR_NOEXCEPT;

    SAtomic32 refcount = 1;
    SAtomic32 weak_refcount = 1;
};

template<typename T, typename Deleter>
struct SRCBlockTyped : public SRCBlock
{
    inline SRCBlockTyped(T* ptr, const Deleter& deleter) SKR_NOEXCEPT
        : ptr(ptr)
        , deleter(deleter)
    {
    }
    inline ~SRCBlockTyped() SKR_NOEXCEPT
    {
        deleter(ptr);
    }
    T* ptr;
    Deleter deleter;
};

template<typename T, typename Deleter>
SRCBlock* SRCBlock::Create(T* ptr, const Deleter& deleter) SKR_NOEXCEPT
{
    return SkrNew<SRCBlockTyped<T, Deleter>>(ptr, deleter);
}

inline void SRCBlock::Destroy(SRCBlock* b) SKR_NOEXCEPT
{
    SkrDelete(b);
}


template <bool Use> struct SRCInst { virtual ~SRCInst() = default; };
template <> struct SRCInst<true> 
{
    virtual ~SRCInst() = default;

    template<typename T, typename Deleter = DefaultDeleter<T>>
    void allocate_block(T* ptr, Deleter deleter);

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

template<typename T, typename Deleter>
void SRCInst<true>::allocate_block(T* ptr, Deleter deleter)
{
    if (block == nullptr)
    {
        block = SRCBlock::Create<T, Deleter>(ptr, deleter);
    }
}
} // namespace skr