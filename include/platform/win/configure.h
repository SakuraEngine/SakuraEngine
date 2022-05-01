#pragma once
#include <stdio.h>
#ifdef _DEBUG
    #include "assert.h"
#else
    #ifndef assert
        #define assert(expr) (void)(expr);
    #endif
#endif

#if defined(_WIN32) || defined(XBOX)
    #define CHECK_HRESULT(exp)                                               \
        do                                                                   \
        {                                                                    \
            HRESULT hres = (exp);                                            \
            if (!SUCCEEDED(hres))                                            \
            {                                                                \
                printf("%s: FAILED with HRESULT: %u", #exp, (uint32_t)hres); \
                assert(false);                                               \
            }                                                                \
        } while (0)
#endif

#define OS_DPI 96

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#if !SKR_SHIPPING
    #include "utils/macros.h"
    #define SKR_DISABLE_OPTIMIZATION __pragma(optimize("", off))
    #define SKR_ENABLE_OPTIMIZATION __pragma(optimize("", on))

    #define SKR_TRACE_MSG(msg)   \
        OutputDebugStringA(msg); \
        OutputDebugStringA("\r\n")
    #define SKR_ASSERT(cond)                                                      \
        do                                                                        \
        {                                                                         \
            if (!(cond))                                                          \
            {                                                                     \
                SKR_TRACE_MSG("Skr Assert fired: " #cond " (" SKR_FILE_LINE ")"); \
                __debugbreak();                                                   \
            }                                                                     \
        } while (0)
    #define SKR_BREAK() __debugbreak()
    #define SKR_HALT() __debugbreak()

#endif