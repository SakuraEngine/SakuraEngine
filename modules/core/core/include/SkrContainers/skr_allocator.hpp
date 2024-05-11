#pragma once
#include "SkrMemory/memory.h"

namespace skr
{
static const char* kContainersDefaultPoolName = "sakura::containers";

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
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "containers::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* p = sakura_malloc_alignedN(size * sizeof(T), alignof(T), kContainersDefaultPoolName);
        SkrCZoneEnd(z);
        return reinterpret_cast<T*>(p);
#else
        return reinterpret_cast<T*>(sakura_malloc_aligned(size * sizeof(T), alignof(T)));
#endif
    }

    template <typename T>
    static void free(T* p)
    {
        if (p)
        {
#if defined(TRACY_TRACE_ALLOCATION)
            SkrCZoneNCS(z, "containers::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
            sakura_free_alignedN(p, alignof(T), kContainersDefaultPoolName);
            SkrCZoneEnd(z);
#else
            sakura_free_aligned(p, alignof(T));
#endif
        }
    }

    template <typename T>
    static T* realloc(T* p, size_t size)
    {
        SkrCZoneNCS(z, "containers::realloc", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* new_mem = sakura_realloc_alignedN(p, size * sizeof(T), alignof(T), kContainersDefaultPoolName);
        SkrCZoneEnd(z);
        return reinterpret_cast<T*>(new_mem);
    }
};
} // namespace skr