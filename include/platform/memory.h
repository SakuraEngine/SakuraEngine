#pragma once
#include "platform/debug.h"

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
    #define sakura_malloc mi_malloc
    #define sakura_calloc mi_calloc
    #define sakura_calloc_aligned mi_calloc_aligned
    #define sakura_malloc_aligned mi_malloc_aligned
    #define sakura_new_aligned mi_new_aligned
    #define sakura_free mi_free
    #define sakura_free_aligned mi_free_aligned
    #define sakura_realloc mi_realloc
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
    #ifdef __cplusplus
        #define sakura_malloc ::malloc
        #define sakura_calloc ::calloc
        #define sakura_calloc_aligned ::calloc_aligned
        #define sakura_malloc_aligned(size, alignment) ::aligned_alloc((alignment), (size))
        #define sakura_new_aligned(size, alignment) ::aligned_alloc((alignment), (size))
        #define sakura_free ::free
        #define sakura_free_aligned ::free_aligned
        #define sakura_realloc ::realloc
    #else
        #define sakura_malloc malloc
        #define sakura_calloc calloc
        #define sakura_calloc_aligned calloc_aligned
        #define sakura_malloc_aligned(size, alignment) aligned_alloc((alignment), (size))
        #define sakura_new_aligned(size, alignment) aligned_alloc((alignment), (size))
        #define sakura_free free
        #define sakura_free_aligned free_aligned
        #define sakura_realloc realloc
    #endif
#endif

#ifdef _CRTDBG_MAP_ALLOC
    #define DEBUG_NEW_SOURCE_LINE (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define DEBUG_NEW_SOURCE_LINE
#endif

#if defined(__cplusplus)
    #include <type_traits>
template <typename T, typename... TArgs>
[[nodiscard]] FORCEINLINE T* SkrNew(TArgs&&... params)
{
    void* pMemory = sakura_malloc_aligned(sizeof(T), alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
}

template <typename T>
[[nodiscard]] FORCEINLINE T* SkrNew()
{
    void* pMemory = sakura_malloc_aligned(sizeof(T), alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
}

template <typename T>
FORCEINLINE void SkrDelete(T* pType)
{
    if (pType != nullptr)
    {
        pType->~T();
        sakura_free((void*)pType);
    }
}
#endif