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

#ifndef RUNTIME_EXPORT
    #if defined(_MSC_VER)
        #define RUNTIME_EXPORT __declspec(dllexport)
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
            memset((var), 0, sizeof(type) * (num));
    #else
        #define DECLARE_ZERO_VLA(type, var, num) \
            type var[(num)];                     \
            memset((var), 0, sizeof(type) * (num));
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
    #else
        #error "Unsupported architecture for gcc compiler"
    #endif

#else
    #error Unknown language dialect
#endif

// Alloc Configure
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
#define sakura_malloc mi_malloc
#define sakura_calloc mi_calloc
#define sakura_calloc_aligned mi_calloc_aligned
#define sakura_malloc_aligned mi_malloc_aligned
#define sakura_free mi_free

// Platform Specific Configure
#ifdef __APPLE__
    #include "apple/configure.h"
#endif
#ifdef _WINDOWS
    #include "win/configure.h"
#endif