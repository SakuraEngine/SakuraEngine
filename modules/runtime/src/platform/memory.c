#include "platform/memory.h"
#include "tracy/TracyC.h"

#if defined(_WIN32)

#ifdef SKR_RUNTIME_USE_MIMALLOC
    RUNTIME_EXTERN_C RUNTIME_API void* mi_malloc(size_t size) SKR_NOEXCEPT;
    RUNTIME_EXTERN_C RUNTIME_API void* mi_calloc(size_t count, size_t size) SKR_NOEXCEPT;
    RUNTIME_EXTERN_C RUNTIME_API void* mi_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT;
    RUNTIME_EXTERN_C RUNTIME_API void* mi_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT;
    RUNTIME_EXTERN_C RUNTIME_API void* mi_new_aligned(size_t size, size_t alignment);
    RUNTIME_EXTERN_C RUNTIME_API void mi_free(void* p) SKR_NOEXCEPT;
    RUNTIME_EXTERN_C RUNTIME_API void mi_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT;
    RUNTIME_EXTERN_C RUNTIME_API void* mi_realloc(void* p, size_t newsize) SKR_NOEXCEPT;
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
    #define mi_malloc_aligned _aligned_malloc
    #define mi_new_aligned _aligned_malloc
    #define mi_free_aligned(p, alignment) _aligned_free((p))
    #define mi_realloc(p, newsize) realloc((p), (newsize))
#endif

#endif

#if defined(_WIN32)
RUNTIME_API void* sakura_malloc(size_t size) SKR_NOEXCEPT
{
    void* p = mi_malloc(size);
    TracyCAlloc(p, size);
    return p;
}

RUNTIME_API void* sakura_calloc(size_t count, size_t size) SKR_NOEXCEPT
{
    void* p = mi_calloc(count, size);
    TracyCAlloc(p, size);
    return p;
}

RUNTIME_API void* sakura_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT
{
    void* p = mi_calloc_aligned(count, size, alignment);
    TracyCAlloc(p, size);
    return p;
}

RUNTIME_API void* sakura_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT
{
    void* p = mi_malloc_aligned(size, alignment);
    TracyCAlloc(p, size);
    return p;
}

RUNTIME_API void* sakura_new_aligned(size_t size, size_t alignment)
{
    void* p = mi_new_aligned(size, alignment);
    TracyCAlloc(p, size);
    return p;
}

RUNTIME_API void sakura_free(void* p) SKR_NOEXCEPT
{
    TracyCFree(p);
    mi_free(p);
}

RUNTIME_API void sakura_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT
{
    TracyCFree(p);
    mi_free_aligned(p, alignment);
}

RUNTIME_API void* sakura_realloc(void* p, size_t newsize) SKR_NOEXCEPT
{
    TracyCFree(p);
    void* np = mi_realloc(p, newsize);
    TracyCAlloc(np, newsize);
    return np;
}

#elif defined(SKR_PLATFORM_WA)

#else
#include <stdlib.h>
#include <string.h>

FORCEINLINE static void* calloc_aligned(size_t count, size_t size, size_t alignment)
{
#if !defined(_WIN32)
    void* ptr = aligned_alloc(alignment, size * count);
#else
    void* ptr = _aligned_malloc(size * count, alignment);
#endif
    memset(ptr, 0, size * count);
    return ptr;
}

RUNTIME_API void* sakura_malloc(size_t size) SKR_NOEXCEPT
{
    return malloc(size);
}

RUNTIME_API void* sakura_calloc(size_t count, size_t size) SKR_NOEXCEPT
{
    return calloc(count, size);
}

RUNTIME_API void* sakura_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT
{
    return calloc_aligned(count, size, alignment);
}

RUNTIME_API void* sakura_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT
{
#if !defined(_WIN32)
    void* ptr = aligned_alloc(size, alignment);
#else
    void* ptr = _aligned_malloc(size, alignment);
#endif
    return ptr;
}

RUNTIME_API void* sakura_new_aligned(size_t size, size_t alignment)
{
#if !defined(_WIN32)
    void* ptr = aligned_alloc(size, alignment);
#else
    void* ptr = _aligned_malloc(size, alignment);
#endif
    return ptr;
}

RUNTIME_API void sakura_free(void* p) SKR_NOEXCEPT
{
    free(p);
}

#define free_aligned(p, alignment) free((p))

RUNTIME_API void sakura_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT
{
    free_aligned(p, alignment);
}

RUNTIME_API void* sakura_realloc(void* p, size_t newsize) SKR_NOEXCEPT
{
    return realloc(p, newsize);
}
#endif