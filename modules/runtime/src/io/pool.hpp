#pragma once
#include "io/io.h"
#include "misc/log.h"
#include <type_traits>

namespace skr {
namespace io {

template<typename I>
struct ISmartPool
{
    virtual SObjectPtr<I> allocate(const uint64_t sequence) SKR_NOEXCEPT = 0;
    virtual void deallocate(I* ptr) SKR_NOEXCEPT = 0;
};

template<typename T, typename I>
struct SmartPool : public ISmartPool<I>
{
    static_assert(std::is_base_of_v<I, T>, "T must be derived from I");

    ~SmartPool()
    {
        if (objcnt != 0)
        {
            SKR_LOG_ERROR("object leaking detected!");
            SKR_ASSERT(0 && "object leaking detected!");
        }
        T* ptr = nullptr;
        while (blocks.try_dequeue(ptr))
        {
            sakura_free_aligned(ptr, alignof(T));
        }
    }

    SObjectPtr<I> allocate(const uint64_t sequence) SKR_NOEXCEPT
    {
        T* ptr = nullptr;
        if (!blocks.try_dequeue(ptr))
        {
            ptr = (T*)sakura_calloc_aligned(1, sizeof(T), alignof(T));
        }
        new (ptr) T(sequence, this);

        skr_atomicu32_add_relaxed(&objcnt, 1);
        return skr::static_pointer_cast<I>(SObjectPtr<T>(ptr));
    }

    void deallocate(I* iptr) SKR_NOEXCEPT
    {
        if (auto ptr = static_cast<T*>(iptr))
        {
            ptr->~T();
            skr_atomicu32_add_relaxed(&objcnt, -1);
            blocks.enqueue(ptr);
        }
    }
    skr::ConcurrentQueue<T*> blocks;
    SAtomicU64 objcnt = 0;
};

} // namespace io
} // namespace skr