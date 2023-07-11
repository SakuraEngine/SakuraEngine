#pragma once
#include "platform/configure.h"

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

#ifdef _DEBUG
    #include "assert.h"
    #define cgpu_assert assert
#else
    #define cgpu_assert(expr) (void)(expr);
#endif
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
#define CGPU_API RUNTIME_API
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