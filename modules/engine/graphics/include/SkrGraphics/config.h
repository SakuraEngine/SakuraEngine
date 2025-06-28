#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/hash.h"
#ifdef __cplusplus
#include "SkrOS/shared_library.hpp"  // IWYU pragma: export
#endif

#define CGPU_USE_VULKAN

#ifdef _WIN32
    #define CGPU_USE_D3D12
#endif
#ifdef __APPLE__
    #define CGPU_USE_METAL
#endif

#if defined(__EMSCRIPTEN__) || defined(__wasi__)
    #ifdef CGPU_USE_VULKAN
        #undef CGPU_USE_VULKAN
    #endif
#endif

#ifdef __cplusplus
    #define CGPU_NULL nullptr
#else
    #define CGPU_NULL 0
#endif

#ifdef __cplusplus
    #ifndef CGPU_NULLPTR
        #define CGPU_NULLPTR nullptr
    #endif
#else
    #ifndef CGPU_NULLPTR
        #define CGPU_NULLPTR CGPU_NULL
    #endif
#endif

#define MAX_GPU_VENDOR_STRING_LENGTH 64
#define MAX_GPU_DEBUG_NAME_LENGTH 128
#define PSO_NAME_LENGTH 160

#ifndef cgpu_max
    #define cgpu_max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef cgpu_min
    #define cgpu_min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// ...
#define CGPU_THREAD_SAFETY

#define cgpu_assert(expr) SKR_ASSERT(expr);
#define cgpu_static_assert static_assert

#if UINTPTR_MAX == UINT32_MAX
#define CGPU_NAME_HASH_SEED 1610612741
#else
#define CGPU_NAME_HASH_SEED 8053064571610612741
#endif
#define cgpu_hash(buffer, size, seed) skr_hash((buffer), (size), (seed))
#define cgpu_name_hash(buffer, size) cgpu_hash((buffer), (size), (CGPU_NAME_HASH_SEED))

#if !defined(ENABLE_NSIGHT_AFTERMATH) && defined(_WIN32)
    #define ENABLE_NSIGHT_AFTERMATH
#endif

#ifndef CGPU_API
#define CGPU_API SKR_GRAPHICS_API
#endif

#ifndef CGPU_EXTERN_C
#ifdef __cplusplus
    #define CGPU_EXTERN_C extern "C"
#else
    #define CGPU_EXTERN_C
#endif
#endif

#ifndef CGPU_EXTERN_C_BEGIN
#ifdef __cplusplus
    #define CGPU_EXTERN_C_BEGIN extern "C" {
#else
    #define CGPU_EXTERN_C_BEGIN
#endif
#endif

#ifndef CGPU_EXTERN_C_END
#ifdef __cplusplus
    #define CGPU_EXTERN_C_END }
#else
    #define CGPU_EXTERN_C_END
#endif
#endif

#ifndef SHIPPING_ONE_ARCHIVE
    #define IMPORT_FROM_RUNTIME SKR_IMPORT
#else
    #define IMPORT_FROM_RUNTIME
#endif

#pragma region LOG

#ifndef CGPU_BUILD_STANDALONE
#include "SkrCore/log.h"

    #define cgpu_backtrace(...) SKR_LOG_BACKTRACE(__VA_ARGS__)
    #define cgpu_trace(...) SKR_LOG_TRACE(__VA_ARGS__)
    #define cgpu_debug(...) SKR_LOG_DEBUG(__VA_ARGS__)
    #define cgpu_info(...) SKR_LOG_INFO(__VA_ARGS__)
    #define cgpu_warn(...) SKR_LOG_WARN(__VA_ARGS__)
    #define cgpu_error(...) SKR_LOG_ERROR(__VA_ARGS__)
    #define cgpu_fatal(...) SKR_LOG_FATAL(__VA_ARGS__)
#else
    #define cgpu_backtrace(...) (void)(__VA_ARGS__)
    #define cgpu_trace(...) (void)(__VA_ARGS__)
    #define cgpu_debug(...) (void)(__VA_ARGS__)
    #define cgpu_info(...) (void)(__VA_ARGS__)
    #define cgpu_warn(...) (void)(__VA_ARGS__)
    #define cgpu_error(...) (void)(__VA_ARGS__)
    #define cgpu_fatal(...) (void)(__VA_ARGS__)
#endif

#pragma endregion LOG

#pragma region MEMORY

#ifdef CGPU_BUILD_STANDALONE
    #include <crtdbg.h>
    
SKR_FORCEINLINE static void* _aligned_calloc(size_t nelem, size_t elsize, size_t alignment)
{
    void* memory = _aligned_malloc(nelem * elsize, alignment);
    if (memory != NULL) memset(memory, 0, nelem * elsize);
    return memory;
}
    #define cgpu_malloc malloc
    #define cgpu_malloc_aligned _aligned_malloc
    #define cgpu_malloc_alignedN(size, alignment, ...)  _aligned_malloc(size, alignment)
    #define cgpu_calloc calloc
    #define cgpu_callocN(count, size, ...) calloc((count), (size))
    #define cgpu_calloc_aligned _aligned_calloc
    #define cgpu_memalign _aligned_malloc
    #define cgpu_free free
    #define cgpu_freeN(ptr, ...) free(ptr)
    #define cgpu_free_aligned _aligned_free
    #define cgpu_free_alignedN(ptr, alignment, ...) _aligned_free((ptr), (alignment))

    #ifdef __cplusplus
    template <typename T, typename... Args>
    T* cgpu_new_placed(void* memory, Args&&... args)
    {
        return new (memory) T(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* cgpu_new(Args&&... args)
    {
        return new T(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* cgpu_new_sized(uint64_t size, Args&&... args)
    {
        void* ptr = cgpu_calloc_aligned(1, size, alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    template <typename T>
    void cgpu_delete_placed(T* object)
    {
        object->~T();
    }

    template <typename T>
    void cgpu_delete(T* object)
    {
        delete object;
    }
    #endif
#else
    #include "SkrCore/memory/memory.h"

    #define cgpu_malloc sakura_malloc
    #define cgpu_malloc_aligned sakura_malloc_aligned
    #define cgpu_malloc_alignedN sakura_malloc_alignedN
    #define cgpu_calloc sakura_calloc
    #define cgpu_callocN sakura_callocN
    #define cgpu_calloc_aligned sakura_calloc_aligned
    #define cgpu_memalign sakura_malloc_aligned
    #define cgpu_free sakura_free
    #define cgpu_freeN sakura_freeN
    #define cgpu_free_aligned(ptr, alignment) sakura_free_aligned((ptr), (alignment))
    #define cgpu_free_alignedN(ptr, alignment, ...) sakura_free_alignedN((ptr), (alignment), __VA_ARGS__)

    #ifdef __cplusplus
    template <typename T, typename... Args>
    T* cgpu_new_placed(void* memory, Args&&... args)
    {
        return new (memory) T(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* cgpu_new(Args&&... args)
    {
        return SkrNewZeroed<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* cgpu_new_sized(uint64_t size, Args&&... args)
    {
        void* ptr = cgpu_calloc_aligned(1, size, alignof(T));
        return cgpu_new_placed<T>(ptr, std::forward<Args>(args)...);
    }

    template <typename T>
    void cgpu_delete_placed(T* object)
    {
        object->~T();
    }

    template <typename T>
    void cgpu_delete(T* object)
    {
        cgpu_delete_placed(object);
        cgpu_free_aligned(object, alignof(T));
    }
    #endif
#endif

#pragma endregion MEMORY