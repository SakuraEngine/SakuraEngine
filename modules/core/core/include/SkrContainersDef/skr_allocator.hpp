#pragma once
#include "SkrCore/memory/memory.h"

namespace skr
{
struct SkrAllocator {
    struct DummyParam {
    };
    using CtorParam                       = DummyParam; // no ctor param
    static constexpr bool support_realloc = true;       // realloc supported

    inline SkrAllocator(DummyParam) noexcept {}
    inline SkrAllocator() noexcept {}
    inline ~SkrAllocator() noexcept {}
    inline SkrAllocator(const SkrAllocator&) {}
    inline SkrAllocator(SkrAllocator&&) noexcept {}
    inline SkrAllocator& operator=(const SkrAllocator&) { return *this; }
    inline SkrAllocator& operator=(SkrAllocator&&) noexcept { return *this; }

    template <typename T>
    inline static T* alloc(size_t size)
    {
        return reinterpret_cast<T*>(alloc_raw(
            size,
            sizeof(T),
            alignof(T)
        ));
    }

    template <typename T>
    inline static void free(T* p)
    {
        return free_raw(
            reinterpret_cast<void*>(p),
            alignof(T)
        );
    }

    template <typename T>
    inline static T* realloc(T* p, size_t size)
    {
        return reinterpret_cast<T*>(realloc_raw(
            reinterpret_cast<void*>(p),
            size,
            sizeof(T),
            alignof(T)
        ));
    }

    //===>raw alloc api
    inline static constexpr const char* kContainersDefaultPoolName = "sakura::containers";

    inline static void* alloc_raw(size_t count, size_t item_size, size_t item_align)
    {
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "containers::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* p = sakura_malloc_alignedN(count * item_size, item_align, kContainersDefaultPoolName);
        SkrCZoneEnd(z);
        return p;
#else
        return sakura_malloc_aligned(count * item_size, item_align);
#endif
    }
    inline static void free_raw(void* p, size_t item_align)
    {
        if (p)
        {
#if defined(TRACY_TRACE_ALLOCATION)
            SkrCZoneNCS(z, "containers::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
            sakura_free_alignedN(p, item_align, kContainersDefaultPoolName);
            SkrCZoneEnd(z);
#else
            sakura_free_aligned(p, item_align);
#endif
        }
    }
    inline static void* realloc_raw(void* p, size_t count, size_t item_size, size_t item_align)
    {
        SkrCZoneNCS(z, "containers::realloc", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* new_mem = sakura_realloc_alignedN(p, count * item_size, item_align, kContainersDefaultPoolName);
        SkrCZoneEnd(z);
        return new_mem;
    }
    //===>raw alloc api
};
} // namespace skr