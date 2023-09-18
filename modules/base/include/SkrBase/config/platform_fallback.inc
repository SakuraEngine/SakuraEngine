#ifdef __SKR_PLATFORM_FALLBACK_INC__
    #error "platform_fallback.inc included twice"
#endif

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

// platform
#ifndef SKR_PLAT_WIN32
    #define SKR_PLAT_WIN32 0
#endif
#ifndef SKR_PLAT_WIN64
    #define SKR_PLAT_WIN64 0
#endif
#ifndef SKR_PLAT_MACOSX
    #define SKR_PLAT_MACOSX 0
#endif
#ifndef SKR_PLAT_UNIX
    #define SKR_PLAT_UNIX 0
#endif
#ifndef SKR_PLAT_IPHONE
    #define SKR_PLAT_IPHONE 0
#endif
#ifndef SKR_PLAT_IPHONE_SIMULATOR
    #define SKR_PLAT_IPHONE_SIMULATOR 0
#endif

// architecture
#ifndef SKR_ARCH_X86
    #define SKR_ARCH_X86 0
#endif
#ifndef SKR_ARCH_X86_64
    #define SKR_ARCH_X86_64 0
#endif
#ifndef SKR_ARCH_ARM32
    #define SKR_ARCH_ARM32 0
#endif
#ifndef SKR_ARCH_ARM64
    #define SKR_ARCH_ARM64 0
#endif
#ifndef SKR_ARCH_POWERPC32
    #define SKR_ARCH_POWERPC32 0
#endif
#ifndef SKR_ARCH_POWERPC64
    #define SKR_ARCH_POWERPC64 0
#endif
#ifndef SKR_ARCH_WA32
    #define SKR_ARCH_WA32 0
#endif
#ifndef SKR_ARCH_WA64
    #define SKR_ARCH_WA64 0
#endif
#ifndef SKR_ARCH_32BIT
    #define SKR_ARCH_32BIT 0
#endif
#ifndef SKR_ARCH_64BIT
    #define SKR_ARCH_64BIT 0
#endif
#ifndef SKR_ARCH_LITTLE_ENDIAN
    #define SKR_ARCH_LITTLE_ENDIAN 0
#endif
#ifndef SKR_ARCH_BIG_ENDIAN
    #define SKR_ARCH_BIG_ENDIAN 0
#endif

// SIMD
#ifndef SKR_ARCH_SSE
    #define SKR_ARCH_SSE 0
#endif
#ifndef SKR_ARCH_SSE2
    #define SKR_ARCH_SSE2 0
#endif
#ifndef SKR_ARCH_AVX
    #define SKR_ARCH_AVX 0
#endif
#ifndef SKR_ARCH_AVX2
    #define SKR_ARCH_AVX2 0
#endif

#define __SKR_PLATFORM_FALLBACK_INC__