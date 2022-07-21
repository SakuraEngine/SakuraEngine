#include "platform/memory.h"
#if !defined(SKR_PLATFORM_WA) && !defined(__APPLE__)
    #ifdef __cplusplus
extern "C" {
    #endif
RUNTIME_API void* mi_malloc(size_t size) SKR_NOEXCEPT;
RUNTIME_API void* mi_calloc(size_t count, size_t size) SKR_NOEXCEPT;
RUNTIME_API void* mi_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT;
RUNTIME_API void* mi_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT;
RUNTIME_API void* mi_new_aligned(size_t size, size_t alignment);
RUNTIME_API void mi_free(void* p) SKR_NOEXCEPT;
RUNTIME_API void mi_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT;
RUNTIME_API void* mi_realloc(void* p, size_t newsize) SKR_NOEXCEPT;
    #ifdef __cplusplus
}
    #endif
#endif

#if !defined(SKR_PLATFORM_WA) && !defined(__APPLE__)
RUNTIME_API void* sakura_malloc(size_t size) SKR_NOEXCEPT
{
    return mi_malloc(size);
}

RUNTIME_API void* sakura_calloc(size_t count, size_t size) SKR_NOEXCEPT
{
    return mi_calloc(count, size);
}

RUNTIME_API void* sakura_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT
{
    return mi_calloc_aligned(count, size, alignment);
}

RUNTIME_API void* sakura_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT
{
    return mi_malloc_aligned(size, alignment);
}

RUNTIME_API void* sakura_new_aligned(size_t size, size_t alignment)
{
    return mi_new_aligned(size, alignment);
}

RUNTIME_API void sakura_free(void* p) SKR_NOEXCEPT
{
    mi_free(p);
}

RUNTIME_API void sakura_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT
{
    mi_free_aligned(p, alignment);
}

RUNTIME_API void* sakura_realloc(void* p, size_t newsize) SKR_NOEXCEPT
{
    return mi_realloc(p, newsize);
}

#elif defined(SKR_PLATFORM_WA)

#else
#include <stdlib.h>
#include <string.h>

FORCEINLINE static void* calloc_aligned(size_t count, size_t size, size_t alignment)
{
    void* ptr = aligned_alloc(alignment, size * count);
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
    return aligned_alloc(size, alignment);
}

RUNTIME_API void* sakura_new_aligned(size_t size, size_t alignment)
{
    return aligned_alloc(size, alignment);
}

RUNTIME_API void sakura_free(void* p) SKR_NOEXCEPT
{
    free(p);
}

RUNTIME_API void sakura_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT
{
    free_aligned(p, alignment);
}

RUNTIME_API void* sakura_realloc(void* p, size_t newsize) SKR_NOEXCEPT
{
    return realloc(p, newsize);
}
#endif