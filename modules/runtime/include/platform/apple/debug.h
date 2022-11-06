#if !defined(SKR_HEADER_SCOPE_DEFINING_PLATFORM_DEBUG)
    #error "this file can only be included in "platform/debug.h"
#endif

#ifndef SKR_APPLE_DEBUG_H
    #define SKR_APPLE_DEBUG_H
#endif
#ifdef SKR_APPLE_DEBUG_H


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