#pragma once
//-------------------------------------------------------------------------------
// -> platform
//      SKR_PLAT_WIN32
//      SKR_PLAT_WIN64
//      SKR_PLAT_WINDOWS
//      SKR_PLAT_MACOSX
//      SKR_PLAT_UNIX
//      SKR_PLAT_IPHONE
//      SKR_PLAT_IPHONE_SIMULATOR
//
// -> architecture
//      SKR_ARCH_X86
//      SKR_ARCH_X86_64
//      SKR_ARCH_ARM32
//      SKR_ARCH_ARM64
//      SKR_ARCH_POWERPC32
//      SKR_ARCH_POWERPC64
//      SKR_ARCH_WA32
//      SKR_ARCH_WA64
//      SKR_ARCH_32BIT
//      SKR_ARCH_64BIT
//      SKR_ARCH_LITTLE_ENDIAN
//      SKR_ARCH_BIG_ENDIAN
//
// -> SIMD
//      SKR_ARCH_SSE
//      SKR_ARCH_SSE2
//      SKR_ARCH_AVX
//      SKR_ARCH_AVX2
//-------------------------------------------------------------------------------

// windows platform
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
    #ifdef _WIN64
        #define SKR_PLAT_WIN64 1
    #else
        #define SKR_PLAT_WIN32 1
    #endif
#endif

// ios platform
#if defined(__APPLE__) && defined(__MACH__)
    /* Apple OSX and iOS (Darwin). */
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
    /* iOS in Xcode simulator */
        #define SKR_PLAT_IPHONE_SIMULATOR 1
    #elif TARGET_OS_IPHONE == 1
    /* iOS */
        #define SKR_PLAT_IPHONE 1
    #elif TARGET_OS_MAC == 1
        /* macOS */
        #define SKR_PLAT_MACOSX 1
    #endif
#endif

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define SKR_PLAT_UNIX 1
#endif

// architecture
#ifndef SKR_MANUAL_CONFIG_CPU_ARCHITECTURE
    #if defined(__x86_64__) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64)
        #define SKR_ARCH_X86_64 1
        #define SKR_ARCH_64BIT 1
        #define SKR_ARCH_LITTLE_ENDIAN 1
        #define SKR_ARCH_SSE 1
        #define SKR_ARCH_SSE2 1
    #elif defined(__i386) || defined(_M_IX86) || defined(_X86_)
        #define SKR_ARCH_X86 1
        #define SKR_ARCH_32BIT 1
        #define SKR_ARCH_LITTLE_ENDIAN 1
        #define SKR_ARCH_SSE 1
        #define SKR_ARCH_SSE2 1
    #elif defined(__aarch64__) || defined(__AARCH64) || defined(_M_ARM64)
        #define SKR_ARCH_ARM64 1
        #define SKR_ARCH_64BIT 1
        #define SKR_ARCH_LITTLE_ENDIAN 1
        #define SKR_ARCH_SSE 1
        #define SKR_ARCH_SSE2 1
    #elif defined(__arm__) || defined(_M_ARM)
        #define SKR_ARCH_ARM32 1
        #define SKR_PLATFORM_32BIT 1
        #define SKR_ARCH_LITTLE_ENDIAN 1
    #elif defined(__POWERPC64__) || defined(__powerpc64__)
        #define SKR_ARCH_POWERPC64 1
        #define SKR_ARCH_64BIT 1
        #define SKR_ARCH_BIG_ENDIAN 1
    #elif defined(__POWERPC__) || defined(__powerpc__)
        #define SKR_ARCH_POWERPC32 1
        #define SKR_PLATFORM_32BIT 1
        #define SKR_ARCH_BIG_ENDIAN 1
    #elif defined(__wasm64__)
        #define SKR_ARCH_WA64 1
        #define SKR_ARCH_64BIT 1
        #define SKR_ARCH_LITTLE_ENDIAN 1
    #elif defined(__wasm__) || defined(__EMSCRIPTEN__) || defined(__wasi__)
        #define SKR_ARCH_WA32 1
        #define SKR_PLATFORM_32BIT 1
        #define SKR_ARCH_LITTLE_ENDIAN 1
    #else
        #error Unrecognized CPU was used.
    #endif
#endif

// SIMD
#if defined(__AVX__)
    #define SKR_ARCH_AVX 1
#endif
#if defined(__AVX2__)
    #define SKR_ARCH_AVX2 1
#endif

// fallback
#include "platform_fallback.inc"

// other platform def
#define SKR_PLAT_WINDOWS SKR_PLAT_WIN32 || SKR_PLAT_WIN64
#define SKR_ARCH_WA SKR_ARCH_WA32 || SKR_ARCH_WA64

// Platform Specific Configure
#define SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE

#ifdef __APPLE__
    #include "apple/configure.inc"
#endif

#ifdef _WIN32
    #include "win/configure.inc"
#endif

#ifndef OS_DPI
    #define OS_DPI 72
#endif

#undef SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE

// TODO: REMOVE THIS
#define SKR_RESOURCE_DEV_MODE