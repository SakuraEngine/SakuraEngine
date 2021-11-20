#pragma once
#include "platform/configure.h"

#define CGPU_USE_VULKAN
#ifdef _WINDOWS
    #define CGPU_USE_D3D12
#endif
#ifdef __APPLE__
    #define CGPU_USE_METAL
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

#ifndef CGPU_MANUAL_CONFIG_CPU_ARCHITECTURE
    #if defined(__x86_64__) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64)
        #define CGPU_PLATFORM_X86_64
    #elif defined(__i386) || defined(_M_IX86) || defined(_X86_)
        #define CGPU_PLATFORM_X86
    #elif defined(__aarch64__) || defined(__AARCH64) || defined(_M_ARM64)
        #define CGPU_PLATFORM_ARM64
    #elif defined(__arm__) || defined(_M_ARM)
        #define CGPU_PLATFORM_ARM32
    #elif defined(__POWERPC64__) || defined(__powerpc64__)
        #define CGPU_PLATFORM_POWERPC64
    #elif defined(__POWERPC__) || defined(__powerpc__)
        #define CGPU_PLATFORM_POWERPC32
    #else
        #error Unrecognized CPU was used.
    #endif
#endif

#ifndef CGPU_MANUAL_CONFIG_CPU_TRAITS
    #if defined(__AVX__)
        #define CGPU_PLATFORM_AVX
    #endif
    #if defined(__AVX2__)
        #define CGPU_PLATFORM_AVX2
    #endif

    #if defined(CGPU_PLATFORM_X86)
        #define CGPU_PLATFORM_32BIT
        #define CGPU_PLATFORM_LITTLE_ENDIAN
        #define CGPU_PLATFORM_SSE
        #define CGPU_PLATFORM_SSE2
    #endif

    #if defined(CGPU_PLATFORM_X86_64)
        #define CGPU_PLATFORM_64BIT
        #define CGPU_PLATFORM_LITTLE_ENDIAN
        #define CGPU_PLATFORM_SSE
        #define CGPU_PLATFORM_SSE2
    #endif

    #if defined(CGPU_PLATFORM_ARM32)
        #define CGPU_PLATFORM_32BIT
        #define CGPU_PLATFORM_LITTLE_ENDIAN
    #endif

    #if defined(CGPU_PLATFORM_ARM64)
        #define CGPU_PLATFORM_64BIT
        #define CGPU_PLATFORM_LITTLE_ENDIAN
        #define CGPU_PLATFORM_SSE
        #define CGPU_PLATFORM_SSE2
    #endif

    #if defined(CGPU_PLATFORM_POWERPC32)
        #define CGPU_PLATFORM_32BIT
        #define CGPU_PLATFORM_BIG_ENDIAN
    #endif

    #if defined(CGPU_PLATFORM_POWERPC64)
        #define CGPU_PLATFORM_64BIT
        #define CGPU_PLATFORM_BIG_ENDIAN
    #endif
#endif

#ifndef CGPU_MANUAL_CONFIG_COMPILER
    #if defined(_MSC_VER)
        #define CGPU_COMPILER_MSVC
    #endif

    #if defined(__clang__)
        #define CGPU_COMPILER_CLANG
    #elif defined(__GNUC__)
        #define CGPU_COMPILER_GCC
    #elif defined(_MSC_VER)
    #else
        #error Unrecognized compiler was used.
    #endif
#endif

#ifndef CGPU_MANUAL_CONFIG_COMPILER_TRAITS
    #if defined(CGPU_COMPILER_MSVC)
        #define CGPU_COMPILER_VERSION _MSC_VER
    #elif defined(CGPU_COMPILER_CLANG)
        #define CGPU_COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)
    #elif defined(CGPU_COMPILER_GCC)
        #define CGPU_COMPILER_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
    #endif
#endif

#ifndef CGPU_MANUAL_CONFIG_CPP_STANDARD
    #if (defined(CGPU_COMPILER_CLANG) || defined(CGPU_COMPILER_GCC))
        #if __cplusplus >= 201703L
            #define CGPU_COMPILER_CPP17
        #endif
        #if __cplusplus >= 201402L
            #define CGPU_COMPILER_CPP14
        #endif
    #elif defined(CGPU_COMPILER_MSVC)
        #if (CGPU_COMPILER_VERSION >= 1920) // VS 2019
            #define CGPU_COMPILER_CPP17
        #endif
        #if (CGPU_COMPILER_VERSION >= 1910) // VS 2017
            #define CGPU_COMPILER_CPP14
        #endif
    #else
        #error "Failed to delect C++ standard version."
    #endif
#endif // CGPU_MANUAL_CONFIG_CPP_STANDARD_VERSION

// no vtable
#ifdef _MSC_VER
    #define CGPU_NOVTABLE __declspec(novtable)
#else
    #define CGPU_NOVTABLE
#endif

// inline defs
#ifndef CGPU_FORCEINLINE
    #ifdef CGPU_COMPILER_MSVC
        #define CGPU_FORCEINLINE __forceinline
    #else
        #define CGPU_FORCEINLINE inline
    #endif
#endif
#define CGPU_INLINE inline
// By Default we use cpp-standard above 2011XXL
#define CGPU_NOEXCEPT noexcept

#ifdef __cplusplus
    #define CGPU_EXTERN_C extern "C"
    #define CGPU_NULL nullptr
#else
    #define CGPU_EXTERN_C
    #define CGPU_NULL 0
#endif

#ifndef CGPU_IL_FUNC
    #define CGPU_IL_FUNC CGPU_FORCEINLINE
#endif

#ifndef CGPU_ZERO_LEN_ARRAY
    #ifdef __GNUC__
        #define CGPU_ZERO_LEN_ARRAY 0
    #else
        #define CGPU_ZERO_LEN_ARRAY 1
    #endif
#endif

#ifndef INTERNAL_CALL
    #define INTERNAL_CALL
#endif

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
extern void* mi_malloc(size_t size);
extern void* mi_calloc(size_t count, size_t size);
extern void* mi_calloc_aligned(size_t count, size_t size, size_t alignment);
extern void* mi_malloc_aligned(size_t size, size_t alignment);
extern void mi_free(void* p);
#ifdef __cplusplus
}
#endif
#define cgpu_malloc mi_malloc
#define cgpu_calloc mi_calloc
#define cgpu_calloc_aligned mi_calloc_aligned
#define cgpu_memalign mi_malloc_aligned
#define cgpu_free mi_free

#define MAX_GPU_VENDOR_STRING_LENGTH 64