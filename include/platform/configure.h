#pragma once
#ifndef __cplusplus
    #include <stdbool.h>
#endif
#include <stdint.h>

#if defined(_MSC_VER)
    #define FORCEINLINE __forceinline
#else
    #define FORCEINLINE inline __attribute__((always_inline))
#endif

#ifdef __cplusplus
    #define RUNTIME_EXTERN_C extern "C"
#else
    #define RUNTIME_EXTERN_C
#endif

#ifndef RUNTIME_EXPORT
    #if defined(_MSC_VER)
        #define RUNTIME_EXPORT __declspec(dllexport) RUNTIME_EXTERN_C
    #else
        #define RUNTIME_EXPORT
    #endif
#endif

#ifndef RUNTIME_API // If the build file hasn't already defined this to be dllexport...
    #ifdef RUNTIME_DLL
        #if defined(_MSC_VER)
            #define RUNTIME_API __declspec(dllimport)
            #define RUNTIME_LOCAL
        #elif defined(__CYGWIN__)
            #define RUNTIME_API __attribute__((dllimport))
            #define RUNTIME_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define RUNTIME_API __attribute__((visibility("default")))
            #define RUNTIME_LOCAL __attribute__((visibility("hidden")))
        #else
            #define RUNTIME_API
            #define RUNTIME_LOCAL
        #endif

        #define EA_DLL
    #else
        #define RUNTIME_API
        #define RUNTIME_LOCAL
    #endif
#endif

#ifndef CHAR8_T_DEFINED // If the user hasn't already defined these...
    #define CHAR8_T_DEFINED
    #if defined(EA_PLATFORM_APPLE)
        #define char8_t char // The Apple debugger is too stupid to realize char8_t is typedef'd to char, so we #define it.
    #else
typedef char char8_t;
    #endif
#endif

#if defined(__cplusplus)
    #define DECLARE_ZERO(type, var) type var = {};
#else
    #define DECLARE_ZERO(type, var) type var = { 0 };
#endif

// VLA
#ifndef __cplusplus
    #if defined(_MSC_VER) && !defined(__clang__)
        #define DECLARE_ZERO_VLA(type, var, num)              \
            type* var = (type*)_alloca(sizeof(type) * (num)); \
            memset((void*)(var), 0, sizeof(type) * (num));
    #else
        #define DECLARE_ZERO_VLA(type, var, num) \
            type var[(num)];                     \
            memset((void*)(var), 0, sizeof(type) * (num));
    #endif
#endif

// PTR SIZE
#if INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
    #define PTR_SIZE 8
#elif INTPTR_MAX == 0x7FFFFFFF
    #define PTR_SIZE 4
#else
    #error unsupported platform
#endif

#if defined(_MSC_VER) && !defined(__clang__)
    #if !defined(_DEBUG)
        #define NDEBUG
    #endif

    #define UNREF_PARAM(x) (x)
    #define ALIGNAS(x) __declspec(align(x))
    #define DEFINE_ALIGNED(def, a) __declspec(align(a)) def
    #define FORGE_CALLCONV __cdecl

    #include <crtdbg.h>
    #define COMPILE_ASSERT(exp) _STATIC_ASSERT(exp)

    #include <BaseTsd.h>
typedef SSIZE_T ssize_t;

    #if defined(_M_X64)
        #define ARCH_X64
        #define ARCH_X86_FAMILY
    #elif defined(_M_IX86)
        #define ARCH_X86
        #define ARCH_X86_FAMILY
    #else
        #error "Unsupported architecture for msvc compiler"
    #endif

#elif defined(__GNUC__) || defined(__clang__)
    #include <sys/types.h>
    #include <assert.h>

    #ifdef __OPTIMIZE__
        // Some platforms define NDEBUG for Release builds
        #ifndef NDEBUG
            #define NDEBUG
        #endif
    #elif !defined(_MSC_VER)
        #define _DEBUG
    #endif

    #ifdef __APPLE__
        #define NOREFS __unsafe_unretained
    #endif

    #define UNREF_PARAM(x) ((void)(x))
    #define ALIGNAS(x) __attribute__((aligned(x)))
    #define DEFINE_ALIGNED(def, a) __attribute__((aligned(a))) def
    #define FORGE_CALLCONV

    #ifdef __clang__
        #define COMPILE_ASSERT(exp) _Static_assert(exp, #exp)
    #else
        #define COMPILE_ASSERT(exp) static_assert(exp, #exp)
    #endif

    #if defined(__i386__)
        #define ARCH_X86
        #define ARCH_X86_FAMILY
    #elif defined(__x86_64__)
        #define ARCH_X64
        #define ARCH_X86_FAMILY
    #elif defined(__arm__)
        #define ARCH_ARM
        #define ARCH_ARM_FAMILY
    #elif defined(__aarch64__)
        #define ARCH_ARM64
        #define ARCH_ARM_FAMILY
    #elif defined(__EMSCRIPTEN__) || defined(__wasi__)
        #define ARCH_WA
        #define ARCH_WEB_FAMILY
    #else
        #error "Unsupported architecture for gcc compiler"
    #endif

#else
    #error Unknown language dialect
#endif

#ifndef RUNTIME_MANUAL_CONFIG_CPU_ARCHITECTURE
    #if defined(__x86_64__) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64)
        #define RUNTIME_PLATFORM_X86_64
    #elif defined(__i386) || defined(_M_IX86) || defined(_X86_)
        #define RUNTIME_PLATFORM_X86
    #elif defined(__aarch64__) || defined(__AARCH64) || defined(_M_ARM64)
        #define RUNTIME_PLATFORM_ARM64
    #elif defined(__arm__) || defined(_M_ARM)
        #define RUNTIME_PLATFORM_ARM32
    #elif defined(__POWERPC64__) || defined(__powerpc64__)
        #define RUNTIME_PLATFORM_POWERPC64
    #elif defined(__POWERPC__) || defined(__powerpc__)
        #define RUNTIME_PLATFORM_POWERPC32
    #elif defined(__EMSCRIPTEN__) || defined(__wasi__)
        #define RUNTIME_PLATFORM_WA
    #else
        #error Unrecognized CPU was used.
    #endif
#endif

#ifndef RUNTIME_MANUAL_CONFIG_CPU_TRAITS
    #if defined(__AVX__)
        #define RUNTIME_PLATFORM_AVX
    #endif
    #if defined(__AVX2__)
        #define RUNTIME_PLATFORM_AVX2
    #endif

    #if defined(RUNTIME_PLATFORM_X86)
        #define RUNTIME_PLATFORM_32BIT
        #define RUNTIME_PLATFORM_LITTLE_ENDIAN
        #define RUNTIME_PLATFORM_SSE
        #define RUNTIME_PLATFORM_SSE2
    #endif

    #if defined(RUNTIME_PLATFORM_X86_64)
        #define RUNTIME_PLATFORM_64BIT
        #define RUNTIME_PLATFORM_LITTLE_ENDIAN
        #define RUNTIME_PLATFORM_SSE
        #define RUNTIME_PLATFORM_SSE2
    #endif

    #if defined(RUNTIME_PLATFORM_ARM32)
        #define RUNTIME_PLATFORM_32BIT
        #define RUNTIME_PLATFORM_LITTLE_ENDIAN
    #endif

    #if defined(RUNTIME_PLATFORM_ARM64)
        #define RUNTIME_PLATFORM_64BIT
        #define RUNTIME_PLATFORM_LITTLE_ENDIAN
        #define RUNTIME_PLATFORM_SSE
        #define RUNTIME_PLATFORM_SSE2
    #endif

    #if defined(RUNTIME_PLATFORM_POWERPC32)
        #define RUNTIME_PLATFORM_32BIT
        #define RUNTIME_PLATFORM_BIG_ENDIAN
    #endif

    #if defined(RUNTIME_PLATFORM_POWERPC64)
        #define RUNTIME_PLATFORM_64BIT
        #define RUNTIME_PLATFORM_BIG_ENDIAN
    #endif
#endif

#ifndef RUNTIME_MANUAL_CONFIG_COMPILER
    #if defined(_MSC_VER)
        #define RUNTIME_COMPILER_MSVC
    #endif

    #if defined(__clang__)
        #define RUNTIME_COMPILER_CLANG
    #elif defined(__GNUC__)
        #define RUNTIME_COMPILER_GCC
    #elif defined(_MSC_VER)
    #else
        #error Unrecognized compiler was used.
    #endif
#endif

#ifndef RUNTIME_MANUAL_CONFIG_COMPILER_TRAITS
    #if defined(RUNTIME_COMPILER_MSVC)
        #define RUNTIME_COMPILER_VERSION _MSC_VER
    #elif defined(RUNTIME_COMPILER_CLANG)
        #define RUNTIME_COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)
    #elif defined(RUNTIME_COMPILER_GCC)
        #define RUNTIME_COMPILER_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
    #endif
#endif

#ifndef RUNTIME_MANUAL_CONFIG_CPP_STANDARD
    #if (defined(RUNTIME_COMPILER_CLANG) || defined(RUNTIME_COMPILER_GCC))
        #if __cplusplus >= 201703L
            #define RUNTIME_COMPILER_CPP17
        #endif
        #if __cplusplus >= 201402L
            #define RUNTIME_COMPILER_CPP14
        #endif
    #elif defined(RUNTIME_COMPILER_MSVC)
        #if (RUNTIME_COMPILER_VERSION >= 1920) // VS 2019
            #define RUNTIME_COMPILER_CPP17
        #endif
        #if (RUNTIME_COMPILER_VERSION >= 1910) // VS 2017
            #define RUNTIME_COMPILER_CPP14
        #endif
    #else
        #error "Failed to delect C++ standard version."
    #endif
#endif // RUNTIME_MANUAL_CONFIG_CPP_STANDARD_VERSION

// no vtable
#ifdef _MSC_VER
    #define RUNTIME_NOVTABLE __declspec(novtable)
#else
    #define RUNTIME_NOVTABLE
#endif

// inline defs
#ifndef RUNTIME_FORCEINLINE
    #ifdef RUNTIME_COMPILER_MSVC
        #define RUNTIME_FORCEINLINE __forceinline
    #else
        #define RUNTIME_FORCEINLINE inline
    #endif
#endif
#define RUNTIME_INLINE inline
// By Default we use cpp-standard above 2011XXL
#define RUNTIME_NOEXCEPT noexcept

// Alloc Configure
#if !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
    #ifdef __cplusplus
extern "C" {
    #endif
RUNTIME_API void* mi_malloc(size_t size);
RUNTIME_API void* mi_calloc(size_t count, size_t size);
RUNTIME_API void* mi_calloc_aligned(size_t count, size_t size, size_t alignment);
RUNTIME_API void* mi_malloc_aligned(size_t size, size_t alignment);
RUNTIME_API void mi_free(void* p);
    #ifdef __cplusplus
}
    #endif
    #define sakura_malloc mi_malloc
    #define sakura_calloc mi_calloc
    #define sakura_calloc_aligned mi_calloc_aligned
    #define sakura_malloc_aligned mi_malloc_aligned
    #define sakura_free mi_free
#else
    #include <stdlib.h>
    #include <string.h>
FORCEINLINE static void* calloc_aligned(size_t count, size_t size, size_t alignment)
{
    void* ptr = aligned_alloc(alignment, size * count);
    memset(ptr, 0, size * count);
    return ptr;
}
    #define sakura_malloc malloc
    #define sakura_calloc calloc
    #define sakura_calloc_aligned calloc_aligned
    #define sakura_malloc_aligned(size, alignment) aligned_alloc((alignment), (size))
    #define sakura_free free
#endif

// Platform Specific Configure
#ifdef __APPLE__
    #include "apple/configure.h"
#endif
#ifdef _WINDOWS
    #include "win/configure.h"
#endif