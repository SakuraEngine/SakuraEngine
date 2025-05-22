#pragma once
#include "SkrCore/log.h"
#include "SkrBase/atomic/atomic.h"
#include "SkrContainers/concurrent_queue.hpp"
#include "SkrCore/memory/rc.hpp"

namespace skr
{

template <typename I>
struct ISmartPool : public skr::IRCAble {
    SKR_RC_IMPL(override);

    virtual ~ISmartPool() SKR_NOEXCEPT = default;
    // template<typename...Args>
    // virtual RC<I> allocate(Args&&... args) SKR_NOEXCEPT = 0;
    virtual void deallocate(I* ptr) SKR_NOEXCEPT = 0;
};
template <typename I>
using ISmartPoolPtr = skr::RC<ISmartPool<I>>;

template <typename T, typename I = T>
struct SmartPool : public ISmartPool<I> {
    static_assert(std::is_base_of_v<I, T>, "T must be derived from I");
    const char* kPoolMemoryPoolName = nullptr;

    SmartPool(const char* PoolMemoryPoolName, uint64_t cnt = 64) SKR_NOEXCEPT
        : kPoolMemoryPoolName(PoolMemoryPoolName)
    {
        for (uint64_t i = 0; i < cnt; ++i)
        {
            blocks.enqueue((T*)sakura_calloc_alignedN(1, sizeof(T), alignof(T), kPoolMemoryPoolName));
        }
    }

    virtual ~SmartPool() SKR_NOEXCEPT
    {
        const auto N = skr_atomic_load_acquire(&objcnt);
        if (N != 0)
        {
            SKR_LOG_ERROR(u8"object leak detected!");
            SKR_ASSERT(0 && u8"object leak detected!");
        }
        T* ptr = nullptr;
        while (blocks.try_dequeue(ptr))
        {
            sakura_free_alignedN(ptr, alignof(T), kPoolMemoryPoolName);
        }
    }

    template <typename... Args>
    RC<I> allocate(Args&&... args) SKR_NOEXCEPT
    {
        T* ptr = nullptr;
        if (!blocks.try_dequeue(ptr))
        {
            ptr = (T*)sakura_calloc_alignedN(1, sizeof(T), alignof(T), kPoolMemoryPoolName);
        }
        new (ptr) T(this, std::forward<Args>(args)...);

        skr_atomic_fetch_add_relaxed(&objcnt, 1);
        return RC<T>(ptr).template cast_static<I>();
    }

    void deallocate(I* iptr) SKR_NOEXCEPT
    {
        if (auto ptr = static_cast<T*>(iptr))
        {
            ptr->~T();
            blocks.enqueue(ptr);
            skr_atomic_fetch_add_relaxed(&objcnt, -1);
        }
        if (recursive_deleting)
            SkrDelete(this);
    }

    bool recursive_deleting = false;
    void skr_rc_delete() override
    {
        const auto N = skr_atomic_load_acquire(&objcnt);
        if (N)
            recursive_deleting = true;
        else
            SkrDelete(this);
    }

    skr::ConcurrentQueue<T*> blocks;
    SAtomic64                objcnt = 0;
};
template <typename T, typename I = T>
using SmartPoolPtr = skr::RC<SmartPool<T, I>>;

} // namespace skr