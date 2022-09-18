#if !defined(SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE)
    #error "this file can only be included in "platform/configure.h"
#endif
#ifndef SKR_APPLE_CONFIGURE_H
    #define SKR_APPLE_CONFIGURE_H
#endif
#ifdef SKR_APPLE_CONFIGURE_H

    #ifdef __cplusplus
extern "C" {
    #endif //  __cplusplus

    #include <TargetConditionals.h>

    /** Building for macOS. */
    #ifndef TARGET_MACOS
        #if (TARGET_OS_OSX || TARGET_OS_MACCATALYST)
            #define TARGET_MACOS
        #endif
    #endif

    /** Building for iOS. */
    #ifndef TARGET_IOS
        #if TARGET_OS_IOS && !TARGET_OS_MACCATALYST
            #define TARGET_IOS
        #endif
    #endif

    /** Building for iOS on Mac Catalyst. */
    #ifndef TARGET_MACCAT
        #if TARGET_OS_MACCATALYST
            #define TARGET_MACCAT
        #endif
    #endif

    /** Building for tvOS. */
    #ifndef TARGET_TVOS
        #if TARGET_OS_TV
            #define TARGET_TVOS
        #endif
    #endif

    /** Building for iOS or tvOS. */
    #ifndef TARGET_IOS_OR_TVOS
        #if defined(TARGET_IOS) || defined(TARGET_TVOS)
            #define TARGET_IOS_OR_TVOS
        #endif
    #endif

    /** Building for macOS or iOS. */
    #ifndef TARGET_MACOS_OR_IOS
        #if defined(TARGET_MACOS) || defined(TARGET_IOS)
            #define TARGET_MACOS_OR_IOS
        #endif
    #endif

    /** Building for a Simulator. */
    #ifndef TARGET_OS_SIMULATOR
        #if TARGET_OS_SIMULATOR
            #define TARGET_OS_SIMULATOR
        #endif
    #endif

    /** Building for Apple Silicon on iOS, tvOS, or macOS platform. */
    #ifndef TARGET_APPLE_SILICON
        #if TARGET_CPU_ARM64
            #define TARGET_APPLE_SILICON
        #endif
    #endif

    /** Building for macOS with support for Apple Silicon. */
    #ifndef TARGET_MACOS_APPLE_SILICON
        #if defined(TARGET_MACOS) && defined(TARGET_APPLE_SILICON)
            #define TARGET_MACOS_APPLE_SILICON
        #endif
    #endif

    /** Building with Xcode versions. */
    #ifndef TARGET_XCODE_13
        #if ((__MAC_OS_X_VERSION_MAX_ALLOWED >= 120000) || (__IPHONE_OS_VERSION_MAX_ALLOWED >= 150000)) // Also covers tvOS
            #define TARGET_XCODE_13
        #endif
    #endif
    #ifndef TARGET_XCODE_12
        #if ((__MAC_OS_X_VERSION_MAX_ALLOWED >= 101600) || (__IPHONE_OS_VERSION_MAX_ALLOWED >= 140000)) // Also covers tvOS
            #define TARGET_XCODE_12
        #endif
    #endif

    #ifdef TARGET_MACOS
        #define _MACOS
    #endif

    #ifdef __cplusplus
}
    #endif //  __cplusplus

    #define OS_DPI 72

    #if !SKR_SHIPPING
        #include "platform/macros.h"
        #include "stdio.h"

        #define SKR_TRACE_MSG(msg) \
            printf(msg);           \
            printf("\n");

        #define SKR_ASSERT(cond)                                                      \
            do                                                                        \
            {                                                                         \
                if (!(cond))                                                          \
                {                                                                     \
                    SKR_TRACE_MSG("Skr Assert fired: " #cond " (" SKR_FILE_LINE ")"); \
                    __builtin_debugtrap();                                            \
                }                                                                     \
            } while (0)
        #define SKR_BREAK() __builtin_debugtrap()
        #define SKR_HALT() __builtin_debugtrap()

    #endif
#endif