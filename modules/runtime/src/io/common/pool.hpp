#pragma once
#include "io/io.h"
#include "misc/log.h"
#include <type_traits>
#include "containers/concurrent_queue.h"

namespace skr {
namespace io {

template<typename I>
struct ISmartPool : public skr::SInterface
{
    virtual ~ISmartPool() = default;
    // template<typename...Args>
    // virtual SObjectPtr<I> allocate(Args&&... args) SKR_NOEXCEPT = 0;
    virtual void deallocate(I* ptr) SKR_NOEXCEPT = 0;

public:
    uint32_t add_refcount() 
    { 
        return 1 + skr_atomicu32_add_relaxed(&rc, 1); 
    }
    uint32_t release() 
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }
private:
    SAtomicU32 rc = 0;
};
template<typename I>
using ISmartPoolPtr = skr::SObjectPtr<ISmartPool<I>>;

extern const char* kIOPoolObjectsMemoryName; 
extern const char* kIOConcurrentQueueName;

template<typename T, typename I>
struct SmartPool : public ISmartPool<I>
{
    static_assert(std::is_base_of_v<I, T>, "T must be derived from I");

    SmartPool(uint64_t cnt = 64)
    {
        for (uint64_t i = 0; i < cnt; ++i)
        {
            blocks.enqueue((T*)sakura_calloc_alignedN(1, sizeof(T), alignof(T), kIOPoolObjectsMemoryName));
        }
    }

    ~SmartPool()
    {
        const auto N = skr_atomic64_load_acquire(&objcnt);
        if (N != 0)
        {
            SKR_LOG_ERROR("object leaking detected!");
            SKR_ASSERT(0 && "object leaking detected!");
        }
        T* ptr = nullptr;
        while (blocks.try_dequeue(ptr))
        {
            sakura_free_alignedN(ptr, alignof(T), kIOPoolObjectsMemoryName);
        }
    }

    template<typename...Args>
    SObjectPtr<I> allocate(Args&&... args) SKR_NOEXCEPT
    {
        T* ptr = nullptr;
        if (!blocks.try_dequeue(ptr))
        {
            ptr = (T*)sakura_calloc_alignedN(1, sizeof(T), alignof(T), kIOPoolObjectsMemoryName);
        }
        else
        {
            memset((void*)ptr, 0, sizeof(T));
        }
        new (ptr) T(this, std::forward<Args>(args)...);

        skr_atomic64_add_relaxed(&objcnt, 1);
        return skr::static_pointer_cast<I>(SObjectPtr<T>(ptr));
    }

    void deallocate(I* iptr) SKR_NOEXCEPT
    {
        if (auto ptr = static_cast<T*>(iptr))
        {
            skr_atomic64_add_relaxed(&objcnt, -1);
            blocks.enqueue(ptr);
            ptr->~T(); // !DO NOT ADD CODE BELOW!
            // CODE ENDS HERE
        }
    }
    skr::ConcurrentQueue<T*> blocks;
    SAtomic64 objcnt = 0;
};
template<typename T, typename I>
using SmartPoolPtr = skr::SObjectPtr<SmartPool<T, I>>;

struct IOConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
{
    static const bool RECYCLE_ALLOCATED_BLOCKS = true;
    static const size_t BLOCK_SIZE = 32;
    static inline void* malloc(size_t size) { return sakura_mallocN(size, kIOConcurrentQueueName); }
    static inline void free(void* ptr) { return sakura_freeN(ptr, kIOConcurrentQueueName); }
};
template<typename T>
using IOConcurrentQueue = moodycamel::ConcurrentQueue<T, IOConcurrentQueueTraits>;  

} // namespace io
} // namespace skr