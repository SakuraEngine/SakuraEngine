#pragma once
#include "platform/configure.h"
#include "platform/memory.h"
// Currently we have only wasm3 as our wasm3 engine
#define USE_M3

#ifdef __cplusplus
    #ifndef SWA_NULLPTR
        #define SWA_NULLPTR nullptr
    #endif
#else
    #ifndef SWA_NULLPTR
        #define SWA_NULLPTR NULL
    #endif
#endif

#ifdef __cplusplus
    #define SWA_EXTERN_C extern "C"
    #define SWA_NULL nullptr
#else
    #define SWA_EXTERN_C
    #define SWA_NULL 0
#endif

#ifdef _DEBUG
    #include "assert.h"
    #define swa_assert assert
#else
    #define swa_assert(expr) (void)(expr);
#endif
#define swa_static_assert static_assert

#define swa_malloc sakura_malloc
#define swa_malloc_aligned sakura_malloc_aligned
#define swa_calloc sakura_calloc
#define swa_calloc_aligned sakura_calloc_aligned
#define swa_memalign sakura_malloc_aligned
#define swa_free sakura_free

#ifdef __cplusplus
    #include <type_traits>
template <typename T, typename... Args>
T* swa_new_placed(void* memory, Args&&... args)
{
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* swa_new(Args&&... args)
{
    void* memory = sakura_malloc_aligned(sizeof(T), alignof(T));
    return swa_new_placed<T>(memory, std::forward<Args>(args)...);
}
template <typename T>
void swa_delete_placed(T* object)
{
    object->~T();
}
template <typename T>
void swa_delete(T* object)
{
    swa_delete_placed(object);
    swa_free(object);
}
#endif

#ifndef swa_max
    #define swa_max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef swa_min
    #define swa_min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#include "utils/hash.h"
#define swa_hash(buffer, size, seed) skr_hash((buffer), (size), (seed))
#include "utils/log.h"
#define swa_trace(...) log_log(SKR_LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define swa_debug(...) log_log(SKR_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define swa_info(...) log_log(SKR_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define swa_warn(...) log_log(SKR_LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define swa_error(...) log_log(SKR_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define swa_fatal(...) log_log(SKR_LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

typedef int64_t swa_i64;
typedef int32_t swa_i32;
typedef double swa_f64;
typedef float swa_f32;
typedef void* swa_ptr;
typedef const void* swa_cptr;


// TODO: Remove this with xmake defines
#ifndef SKR_WASM_EXPORT
    #if defined(_MSC_VER)
        #define SKR_WASM_EXPORT __declspec(dllexport)
    #else
        #define SKR_WASM_EXPORT
    #endif
#endif

#ifdef SKR_WASM_IMPL
    #ifndef SKR_WASM_API
        #define SKR_WASM_API SKR_WASM_EXPORT
    #endif
#endif

#ifndef SKR_WASM_API // If the build file hasn't already defined this to be dllexport...
    #ifdef SKR_WASM_SHARED
        #if defined(_MSC_VER)
            #define SKR_WASM_API __declspec(dllimport)
            #define SKR_WASM_LOCAL
        #elif defined(__CYGWIN__)
            #define SKR_WASM_API __attribute__((dllimport))
            #define SKR_WASM_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define SKR_WASM_API __attribute__((visibility("default")))
            #define SKR_WASM_LOCAL __attribute__((visibility("hidden")))
        #else
            #define SKR_WASM_API
            #define SKR_WASM_LOCAL
        #endif
    #else
        #define SKR_WASM_API
        #define SKR_WASM_LOCAL
    #endif
#endif