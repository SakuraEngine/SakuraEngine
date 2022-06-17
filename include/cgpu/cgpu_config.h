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
    #define CGPU_EXTERN_C extern "C"
    #define CGPU_NULL nullptr
#else
    #define CGPU_EXTERN_C
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

#define cgpu_hash(buffer, size, seed) skr_hash((buffer), (size), (seed))