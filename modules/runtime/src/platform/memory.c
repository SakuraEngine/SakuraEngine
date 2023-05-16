#include "platform/memory.h"
#include "tracy/TracyC.h"

const char* kTracedNewDefaultPoolName = "sakura::default";

#if defined(_WIN32)

#ifdef SKR_RUNTIME_USE_MIMALLOC
    #include "mimalloc/mimalloc.h"
#else
#include <malloc.h>
#include <string.h>
    inline static void* mi_malloc(size_t size)
    {
        return malloc(size);
    }
    inline static void* mi_calloc(size_t count, size_t size)
    {
        void* ptr = calloc(count, size);
        return ptr;
    }
    inline static void* mi_calloc_aligned(size_t count, size_t size, size_t alignment)
    {
        void* ptr = _aligned_malloc(count * size, alignment);
        size_t msize = _aligned_msize(ptr, alignment, 0);
        memset(ptr, 0, msize);
        return ptr;
    }
    inline static void mi_free(void* ptr)
    {
        free(ptr);
    }
    inline static void* mi_new_n(size_t count, size_t size)
    {
        return mi_malloc(count * size);
    }
    #define mi_malloc_aligned _aligned_malloc
    #define mi_new_aligned _aligned_malloc
    #define mi_free_aligned(p, alignment) _aligned_free((p))
    #define mi_realloc(p, newsize) realloc((p), (newsize))
    #define mi_realloc_aligned(p, newsize, alignment) realloc((p), (newsize))
#endif

#endif

// traced_os_alooc
#include <stdlib.h>
#include <string.h>

FORCEINLINE static void* calloc_aligned(size_t count, size_t size, size_t alignment)
{
#if !defined(_WIN32)
    void* ptr = (alignment == 1) ? malloc(size) : aligned_alloc(alignment, size * count);
    if (!ptr)
    {
        posix_memalign(&ptr, alignment, size);
    }
#else
    void* ptr = _aligned_malloc(size * count, alignment);
#endif
    memset(ptr, 0, size * count);
    return ptr;
}

static const char* kDefaultOSAllocPoolName = "sakura::os_alloc";

RUNTIME_API void* traced_os_malloc(size_t size, const char* pool_name) 
{
    void* ptr = malloc(size);
    TracyCAllocN(ptr, size, pool_name ? pool_name : kDefaultOSAllocPoolName);
    return ptr;
}

RUNTIME_API void* traced_os_calloc(size_t count, size_t size, const char* pool_name) 
{
    void* ptr = calloc(count, size);
    TracyCAllocN(ptr, size, pool_name ? pool_name : kDefaultOSAllocPoolName);
    return ptr;
}

RUNTIME_API void* traced_os_calloc_aligned(size_t count, size_t size, size_t alignment, const char* pool_name) 
{
    void* ptr = calloc_aligned(count, size, alignment);
    TracyCAllocN(ptr, size, pool_name ? pool_name : kDefaultOSAllocPoolName);
    return ptr;
}

RUNTIME_API void* traced_os_malloc_aligned(size_t size, size_t alignment, const char* pool_name) 
{
#if !defined(_WIN32)
    void* ptr = (alignment == 1) ? malloc(size) : aligned_alloc(alignment, size);
    if (!ptr)
    {
        posix_memalign(&ptr, alignment, size);
    }
#else
    void* ptr = _aligned_malloc(size, alignment);
#endif
    TracyCAllocN(ptr, size, pool_name ? pool_name : kDefaultOSAllocPoolName);
    return ptr;
}

RUNTIME_API void traced_os_free(void* p, const char* pool_name) 
{
    free(p);
    TracyCFreeN(p, pool_name ? pool_name : kDefaultOSAllocPoolName);
}

#if !defined(_WIN32)
#define os_free_aligned(p, alignment) free((p))
#else
#define os_free_aligned(p, alignment) _aligned_free((p))
#endif

RUNTIME_API void traced_os_free_aligned(void* p, size_t alignment, const char* pool_name) 
{
    os_free_aligned(p, alignment);
    TracyCFreeN(p, pool_name ? pool_name : kDefaultOSAllocPoolName);
}

RUNTIME_API void* traced_os_realloc(void* p, size_t newsize, const char* pool_name) 
{
    TracyCFreeN(p, pool_name ? pool_name : kDefaultOSAllocPoolName);
    void* ptr = realloc(p, newsize);
    TracyCAllocN(ptr, newsize, pool_name ? pool_name : kDefaultOSAllocPoolName);
    return ptr;
}

RUNTIME_EXTERN_C RUNTIME_API void* traced_os_realloc_aligned(void* p, size_t newsize, size_t alignment, const char* pool_name)
{
#if defined(_WIN32)
    TracyCFreeN(p, pool_name ? pool_name : kDefaultOSAllocPoolName);
    void* ptr = _aligned_realloc(p, newsize, alignment);
    TracyCAllocN(ptr, newsize, pool_name ? pool_name : kDefaultOSAllocPoolName);
    return ptr;
#endif
    // There is no posix_memalign_realloc or something like that on posix. But usually realloc will
    // allocated with enough alignment for most `align` values we will use. So we try to realloc first
    // plainly. If the returned address is not aligned adequately, we will revert to
    // posix_memalign'ing a new block, copying old data, deallocating old memory.
    alignment = alignment < _Alignof(void *) ? _Alignof(void *) : alignment;
    void *new_allocation = traced_os_realloc(p, newsize, pool_name);
    if ((uint64_t)(new_allocation) % alignment != 0) {
        //log_warn("This is slow. realloc did not allocate with an alignment of %lu, requires double copy",
        //            (long unsigned int)align);
        void *new_new = traced_os_malloc_aligned(newsize, alignment, pool_name);
        memcpy(new_new, new_allocation, newsize);
        traced_os_free(new_allocation, pool_name);
        new_allocation = new_new;
    }
    return new_allocation;
}

// _sakura_alloc

#if defined(_WIN32)
RUNTIME_API void* _sakura_malloc(size_t size, const char* pool_name) 
{
    void* p = mi_malloc(size);
    if (pool_name)
    {
        TracyCAllocN(p, size, pool_name);
    }
    else
    {
        TracyCAlloc(p, size);
    }
    return p;
}

RUNTIME_API void* _sakura_calloc(size_t count, size_t size, const char* pool_name) 
{
    void* p = mi_calloc(count, size);
    if (pool_name)
    {
        TracyCAllocN(p, size, pool_name);
    }
    else
    {
        TracyCAlloc(p, size);
    }
    return p;
}

RUNTIME_API void* _sakura_calloc_aligned(size_t count, size_t size, size_t alignment, const char* pool_name) 
{
    void* p = mi_calloc_aligned(count, size, alignment);
    if (pool_name)
    {
        TracyCAllocN(p, size, pool_name);
    }
    else
    {
        TracyCAlloc(p, size);
    }
    return p;
}

RUNTIME_API void* _sakura_malloc_aligned(size_t size, size_t alignment, const char* pool_name) 
{
    void* p = mi_malloc_aligned(size, alignment);
    if (pool_name)
    {
        TracyCAllocN(p, size, pool_name);
    }
    else
    {
        TracyCAlloc(p, size);
    }
    return p;
}

RUNTIME_EXTERN_C RUNTIME_API void* _sakura_new_n(size_t count, size_t size, const char* pool_name) 
{
    void* p = mi_new_n(count, size);
    if (pool_name)
    {
        TracyCAllocN(p, size, pool_name);
    }
    else
    {
        TracyCAlloc(p, size);
    }
    return p;
}

RUNTIME_API void* _sakura_new_aligned(size_t size, size_t alignment, const char* pool_name) 
{
    void* p = mi_new_aligned(size, alignment);
    if (pool_name)
    {
        TracyCAllocN(p, size, pool_name);
    }
    else
    {
        TracyCAlloc(p, size);
    }
    return p;
}

RUNTIME_API void _sakura_free(void* p, const char* pool_name) 
{
    if (pool_name)
    {
        TracyCFreeN(p, pool_name);
    }
    else
    {
        TracyCFree(p);
    }
    mi_free(p);
}

RUNTIME_API void _sakura_free_aligned(void* p, size_t alignment, const char* pool_name) 
{
    if (pool_name)
    {
        TracyCFreeN(p, pool_name);
    }
    else
    {
        TracyCFree(p);
    }
    mi_free_aligned(p, alignment);
}

RUNTIME_API void* _sakura_realloc(void* p, size_t newsize, const char* pool_name) 
{
    if (pool_name)
    {
        TracyCFreeN(p, pool_name);
    }
    else
    {
        TracyCFree(p);
    }
    void* np = mi_realloc(p, newsize);
    if (pool_name)
    {
        TracyCAllocN(np, newsize, pool_name);
    }
    else
    {
        TracyCAlloc(np, newsize);
    }
    return np;
}

#elif defined(SKR_PLATFORM_WA)

#else

RUNTIME_API void* _sakura_malloc(size_t size, const char* pool_name) 
{
    return traced_os_malloc(size, pool_name);
}

RUNTIME_API void* _sakura_calloc(size_t count, size_t size, const char* pool_name) 
{
    return traced_os_calloc(count, size, pool_name);
}

RUNTIME_EXTERN_C RUNTIME_API void* _sakura_new_n(size_t count, size_t size, const char* pool_name)
{
    void* p = malloc(count * size);
    return p;
}

RUNTIME_API void* _sakura_calloc_aligned(size_t count, size_t size, size_t alignment, const char* pool_name) 
{
    return traced_os_calloc_aligned(count, size, alignment, pool_name);
}

RUNTIME_API void* _sakura_malloc_aligned(size_t size, size_t alignment, const char* pool_name) 
{
    return traced_os_malloc_aligned(size, alignment, pool_name);
}

RUNTIME_API void* _sakura_new_aligned(size_t size, size_t alignment, const char* pool_name)
{
    return traced_os_malloc_aligned(size, alignment, pool_name);
}

RUNTIME_API void _sakura_free(void* p, const char* pool_name) 
{
    return traced_os_free(p, pool_name);
}

RUNTIME_API void _sakura_free_aligned(void* p, size_t alignment, const char* pool_name) 
{
    traced_os_free_aligned(p, alignment, pool_name);
}

RUNTIME_API void* _sakura_realloc(void* p, size_t newsize, const char* pool_name) 
{
    return traced_os_realloc(p, newsize, pool_name);
}

#endif

const char* kContainersDefaultPoolName = "sakura::containers";

void* containers_malloc_aligned(size_t size, size_t alignment)
{
#if defined(TRACY_TRACE_ALLOCATION)
    TracyCZoneNCS(z, "containers::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
    void* p = _sakura_malloc_aligned(size, alignment, kContainersDefaultPoolName);
    TracyCZoneEnd(z);
    return p;
#else
    return sakura_malloc_aligned(size, alignment);
#endif
}

void containers_free_aligned(void* p, size_t alignment)
{
#if defined(TRACY_TRACE_ALLOCATION)
    TracyCZoneNCS(z, "containers::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
    _sakura_free_aligned(p, alignment, kContainersDefaultPoolName);
    TracyCZoneEnd(z);
#else
    sakura_free_aligned(p, alignment);
#endif
}