#ifdef __SKR_PLATFORM_FALLBACK_INC__
    #error "platform_fallback.inc included twice"
#endif

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

// platform
#ifndef SKR_PLAT_WIN32
    #define SKR_PLAT_WIN320
#endif
#ifndef SKR_PLAT_WIN64
    #define SKR_PLAT_WIN64 0
#endif
#ifndef SKR_PLAT_MAC_OSX
    #define SKR_PLAT_MAC_OSX 0
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

#define __SKR_PLATFORM_FALLBACK_INC__