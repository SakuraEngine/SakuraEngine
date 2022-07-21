#pragma once
#include "platform/configure.h"
#include "platform/debug.h"

RUNTIME_EXTERN_C RUNTIME_API void* sakura_malloc(size_t size) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_calloc(size_t count, size_t size) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_new_aligned(size_t size, size_t alignment);
RUNTIME_EXTERN_C RUNTIME_API void sakura_free(void* p) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void sakura_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_realloc(void* p, size_t newsize) SKR_NOEXCEPT;

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