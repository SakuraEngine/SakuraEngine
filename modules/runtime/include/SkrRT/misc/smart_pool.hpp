#pragma once
#include "SkrRT/misc/log.h"
#include "SkrRT/platform/atomic.h"
#include "SkrRT/containers/sptr.hpp"
#include "SkrRT/containers/concurrent_queue.h"

namespace skr {

template<typename I>
struct ISmartPool : public skr::SInterface
{
    virtual ~ISmartPool() SKR_NOEXCEPT = default;
    // template<typename...Args>
    // virtual SObjectPtr<I> allocate(Args&&... args) SKR_NOEXCEPT = 0;
    virtual void deallocate(I* ptr) SKR_NOEXCEPT = 0;

public:
    inline uint32_t add_refcount() SKR_NOEXCEPT
    { 
        return 1 + skr_atomicu32_add_relaxed(&rc, 1); 
    }
    inline uint32_t release() SKR_NOEXCEPT
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }
private:
    SAtomicU32 rc = 0;
};
template<typename I>
using ISmartPoolPtr = skr::SObjectPtr<ISmartPool<I>>;

template<typename T, typename I = T>
struct SmartPool : public ISmartPool<I>
{
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
        const auto N = skr_atomic64_load_acquire(&objcnt);
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

    template<typename...Args>
    SObjectPtr<I> allocate(Args&&... args) SKR_NOEXCEPT
    {
        T* ptr = nullptr;
        if (!blocks.try_dequeue(ptr))
        {
            ptr = (T*)sakura_calloc_alignedN(1, sizeof(T), alignof(T), kPoolMemoryPoolName);
        }
        new (ptr) T(this, std::forward<Args>(args)...);

        skr_atomic64_add_relaxed(&objcnt, 1);
        return skr::static_pointer_cast<I>(SObjectPtr<T>(ptr));
    }

    void deallocate(I* iptr) SKR_NOEXCEPT
    {
        if (auto ptr = static_cast<T*>(iptr))
        {
            ptr->~T(); 
            blocks.enqueue(ptr);
            skr_atomic64_add_relaxed(&objcnt, -1);
        }
        if (recursive_deleting)
            SkrDelete(this);
    }

    bool recursive_deleting = false;
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<SmartPool<T, I>*>(ptr);
            const auto N = skr_atomic64_load_acquire(&p->objcnt);
            if (N)
                p->recursive_deleting = true;
            else
                SkrDelete(p);
        };
    }

    skr::ConcurrentQueue<T*> blocks;
    SAtomic64 objcnt = 0;
};
template<typename T, typename I = T>
using SmartPoolPtr = skr::SObjectPtr<SmartPool<T, I>>;

} // namespace skr

#define SKR_RC_OBJECT_BODY \
private:\
    SAtomicU32 rc = 0;\
public:\
    uint32_t add_refcount() final\
    {\
        return 1 + skr_atomicu32_add_relaxed(&rc, 1);\
    }\
    uint32_t release() final\
    {\
        skr_atomicu32_add_relaxed(&rc, -1);\
        return skr_atomicu32_load_acquire(&rc);\
    }