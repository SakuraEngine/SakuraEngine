#if !defined(SKR_HEADER_SCOPE_DEFINING_PLATFORM_DEBUG)
    #error "this file can only be included in "platform/debug.h"
#endif
#ifndef SKR_WINDOWS_DEBUG_H
    #define SKR_WINDOWS_DEBUG_H
#endif
#ifdef SKR_WINDOWS_DEBUG_H
    #include <stdlib.h>
    #include <stdio.h>

    #ifdef _DEBUG
        #include <assert.h>
    #else
        #ifndef assert
            #define assert(expr) (void)(expr);
        #endif
    #endif

    #if defined(_WIN32) || defined(XBOX)
        #define CHECK_HRESULT(exp)                                               \
            do                                                                   \
            {                                                                    \
                HRESULT __hres = (exp);                                            \
                if (!SUCCEEDED(__hres))                                            \
                {                                                                \
                    printf("%s: FAILED with HRESULT: %u\n", #exp, (uint32_t)__hres); \
                    assert(false);                                               \
                }                                                                \
            } while (0)
    #endif

    #if !SKR_SHIPPING
        #define SKR_TRACE_MSG(msg) \
            printf(msg);           \
            printf("\n");
        
        #ifndef SKR_ASSERT
        #define SKR_ASSERT(cond)                                                      \
            do                                                                        \
            {                                                                         \
                if (!(cond))                                                          \
                {                                                                     \
                    SKR_TRACE_MSG("Skr Assert fired: " #cond " (" SKR_FILE_LINE ")"); \
                    assert((cond));                                                   \
                }                                                                     \
            } while (0)
        #endif
        
        #define SKR_BREAK() __debugbreak()
        #define SKR_HALT() __debugbreak()

    #endif
#endif