#if !defined(SKR_HEADER_SCOPE_DEFINING_PLATFORM_DEBUG)
    #error "this file can only be included in "platform/debug.h"
#endif

#ifndef SKR_APPLE_DEBUG_H
    #define SKR_APPLE_DEBUG_H
#endif
#ifdef SKR_APPLE_DEBUG_H

    RUNTIME_API void apple_assert_handler(int sig);

    #if !SKR_SHIPPING
        #include "misc/macros.h"
        #include <stdio.h>
        #include <signal.h>

        #define SKR_TRACE_MSG(msg) \
            printf(msg);           \
            printf("\n");

        #define SKR_ASSERT(cond)                                                      \
            do                                                                        \
            {                                                                         \
                if (!(cond))                                                          \
                {                                                                     \
                    SKR_TRACE_MSG("Skr Assert fired: " #cond " (" SKR_FILE_LINE ")"); \
                    signal(SIGSEGV, apple_assert_handler);\
                    __builtin_debugtrap();                                            \
                }                                                                     \
            } while (0)
        #define SKR_BREAK() __builtin_debugtrap()
        #define SKR_HALT() __builtin_debugtrap()

    #endif


#endif