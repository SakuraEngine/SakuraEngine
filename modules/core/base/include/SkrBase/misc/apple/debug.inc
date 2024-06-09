#if !defined(SKR_HEADER_SCOPE_DEFINING_PLATFORM_DEBUG)
    #error "this file can only be included in "platform/debug.h"
#endif

#ifndef SKR_APPLE_DEBUG_H
    #define SKR_APPLE_DEBUG_H
#endif
#ifdef SKR_APPLE_DEBUG_H

    SKR_EXTERN_C SKR_STATIC_API void apple_assert_handler(int sig);

    #if !SKR_SHIPPING
        #include <assert.h>
        #include <stdio.h>
        #include <stdlib.h>
        #include <signal.h>

        inline static void __signal(int sig, void (*handler)(int))
        {
            signal(sig, handler);
        }

        #define SKR_TRACE_MSG(msg) \
            printf(msg);           \
            printf("\n");

        #define SKR_ASSERT(cond)                                                      \
            do                                                                        \
            {                                                                         \
                if (!(cond))                                                          \
                {                                                                     \
                    SKR_TRACE_MSG("Skr Assert fired: " #cond " (" SKR_FILE_LINE ")"); \
                    __signal(SIGSEGV, apple_assert_handler);                            \
                    assert((cond));                                                   \
                }                                                                     \
            } while (0)
        #define SKR_BREAK() __builtin_debugtrap()
        #define SKR_HALT() __builtin_debugtrap()

    #endif


#endif
