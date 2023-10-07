#pragma once
#include "SkrRT/platform/memory.h"
#include "SkrBase/containers/allocator/allocator.hpp"

namespace skr
{
static const char* kContainersDefaultPoolName = "sakura::containers";

struct SkrAllocator : container::AllocTemplate<SkrAllocator, size_t> {
    static void* alloc_raw(size_t size, size_t alignment)
    {
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "containers::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* p = _sakura_malloc_aligned(size, alignment, kContainersDefaultPoolName);
        SkrCZoneEnd(z);
        return p;
#else
        return sakura_malloc_aligned(size, alignment);
#endif
    }
    static void free_raw(void* p, size_t alignment)
    {
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "containers::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        _sakura_free_aligned(p, alignment, kContainersDefaultPoolName);
        SkrCZoneEnd(z);
#else
        sakura_free_aligned(p, alignment);
#endif
    }
    static void* realloc_raw(void* p, size_t size, size_t alignment)
    {
        SkrCZoneNCS(z, "containers::realloc", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        void* new_mem = traced_os_malloc_aligned(size, alignment, kContainersDefaultPoolName);
        if (p)
        {
            memcpy(new_mem, p, size);
            traced_os_free_aligned(p, alignment, kContainersDefaultPoolName);
        }
        SkrCZoneEnd(z);
        return new_mem;
    }
};
} // namespace skr