#pragma once
#include "SkrCore/memory/memory.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrContainersDef/set.hpp"
#include "SkrContainersDef/concurrent_queue.hpp"

namespace skr::ecs
{

struct SKR_RUNTIME_API StackAllocator {
    struct CtorParam {  };
    
    static constexpr bool support_realloc = false; // Pool allocators don't support realloc
    
    StackAllocator(CtorParam param) noexcept;
    ~StackAllocator() noexcept;
    
    template <typename T>
    inline static T* alloc(size_t count)
    {
        return reinterpret_cast<T*>(alloc_raw(count, sizeof(T), alignof(T)));
    }
    
    template <typename T>
    inline static void free(T* p)
    {
        if (p) {
            free_raw(reinterpret_cast<void*>(p), alignof(T));
        }
    }
    
    template <typename T>
    inline static T* realloc(T* p, size_t size)
    {
        return nullptr;
    }
    
    // Raw allocation API
    static void* alloc_raw(size_t count, size_t item_size, size_t item_align);
    static void free_raw(void* p, size_t item_align);
    
    static void Initialize();
    static void Finalize();
    static void Reset();
    
    struct Impl;
};

template <typename T>
using StackVector = skr::Vector<T, StackAllocator>;

template <typename K, typename V>
using StackMap = skr::Map<K, V, skr::container::HashTraits<K>, StackAllocator>;

template <typename K>
using StackSet = skr::Set<K, skr::container::HashTraits<K>, StackAllocator>;

struct StackAllocatorConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
{
    static const bool RECYCLE_ALLOCATED_BLOCKS = true;
    static inline void* malloc(size_t size) { return StackAllocator::alloc_raw(1, size, 16); }
    static inline void free(void* ptr) { return StackAllocator::free_raw(ptr, 16); }
};

template <typename T>
using StackConcurrentQueue = skr::ConcurrentQueue<T, StackAllocatorConcurrentQueueTraits>;

} // namespace skr::ecs