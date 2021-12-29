#pragma once
#include "platform/configure.h"

#define CGPU_USE_VULKAN

#ifdef _WINDOWS
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
    #ifndef CGPU_NULLPTR
        #define CGPU_NULLPTR nullptr
    #endif
#else
    #ifndef CGPU_NULLPTR
        #define CGPU_NULLPTR NULL
    #endif
#endif

#ifdef __cplusplus
    #define CGPU_EXTERN_C extern "C"
    #define CGPU_NULL nullptr
#else
    #define CGPU_EXTERN_C
    #define CGPU_NULL 0
#endif

#define MAX_GPU_VENDOR_STRING_LENGTH 64
#define MAX_GPU_DEBUG_NAME_LENGTH 128

#ifndef cgpu_max
    #define cgpu_max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef cgpu_min
    #define cgpu_min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// ...
#define cgpu_malloc sakura_malloc
#define cgpu_malloc_aligned sakura_malloc_aligned
#define cgpu_calloc sakura_calloc
#define cgpu_calloc_aligned sakura_calloc_aligned
#define cgpu_memalign sakura_malloc_aligned
#define cgpu_free sakura_free

//#define CGPU_THREAD_SAFETY

#ifdef _DEBUG
    #include "assert.h"
    #define cgpu_assert assert
#else
    #define cgpu_assert(expr) (void)(expr);
#endif
#define cgpu_static_assert static_assert

#ifdef __cplusplus
    #include <type_traits>
template <typename T, typename... Args>
T* cgpu_new_placed(void* memory, Args&&... args)
{
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* cgpu_new(Args&&... args)
{
    void* memory = sakura_malloc_aligned(sizeof(T), alignof(T));
    return cgpu_new_placed<T>(memory, std::forward<Args>(args)...);
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
    cgpu_free(object);
}
#endif

#include "utils/hash.h"
#define cgpu_hash(buffer, size, seed) skr_hash((buffer), (size), (seed))
#include "utils/log.h"
#define cgpu_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define cgpu_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
