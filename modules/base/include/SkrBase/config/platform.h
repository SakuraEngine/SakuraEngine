#pragma once
//-------------------------------------------------------------------------------
// -> platform
//      SKR_PLAT_WIN32
//      SKR_PLAT_WIN64
//      SKR_PLAT_WINDOWS
//      SKR_PLAT_MAC_OSX
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
        #define SKR_PLAT_IPHONE_SIMULATOR
    #elif TARGET_OS_IPHONE == 1
    /* iOS */
        #define SKR_PLAT_IPHONE
    #elif TARGET_OS_MAC == 1
        /* macOS */
        #define SKR_PLAT_MAC_OSX
    #endif
#endif

// architecture
#ifndef SKR_MANUAL_CONFIG_CPU_ARCHITECTURE
    #if defined(__x86_64__) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64)
        #define SKR_ARCH_X86_64
    #elif defined(__i386) || defined(_M_IX86) || defined(_X86_)
        #define SKR_ARCH_X86
    #elif defined(__aarch64__) || defined(__AARCH64) || defined(_M_ARM64)
        #define SKR_ARCH_ARM64
    #elif defined(__arm__) || defined(_M_ARM)
        #define SKR_ARCH_ARM32
    #elif defined(__POWERPC64__) || defined(__powerpc64__)
        #define SKR_ARCH_POWERPC64
    #elif defined(__POWERPC__) || defined(__powerpc__)
        #define SKR_ARCH_POWERPC32
    #elif defined(__wasm64__)
        #define SKR_ARCH_WA64
    #elif defined(__wasm__) || defined(__EMSCRIPTEN__) || defined(__wasi__)
        #define SKR_ARCH_WA32
    #else
        #error Unrecognized CPU was used.
    #endif
#endif

#include "platform_fallback.inc"
#define SKR_PLAT_WINDOWS SKR_PLAT_WIN32 || SKR_PLAT_WIN64
#define SKR_ARCH_WA SKR_ARCH_WA32 || SKR_ARCH_WA64