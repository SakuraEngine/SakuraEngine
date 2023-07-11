#pragma once
//-------------------------------------------------------------------------------
// -> platform
//      SKR_PLAT_WIN32
//      SKR_PLAT_WIN64
//
// -> architecture
//      SKR_ARCH_X86
//      SKR_ARCH_X86_64
//
//-------------------------------------------------------------------------------

// windows
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
    // platform
    #ifdef _WIN64
        #define SKR_PLAT_WIN64 1
    #else
        #define SKR_PLAT_WIN32 1
    #endif

    // architecture
    #if defined(_M_IX86) || defined(_X86_)
        #define SKR_ARCH_X86 1
    #elif defined(_M_AMD64) || defined(_AMD64_) || defined(__x86_64__)
        #define SKR_ARCH_X86_64 1
    #else
        #error Unknown processor
        #error Unknown endianness
    #endif
#endif

#include "platform_fallback.inc"
#define SKR_PLAT_WINDOWS SKR_PLAT_WIN32 || SKR_PLAT_WIN64