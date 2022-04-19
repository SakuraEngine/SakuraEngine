#pragma once
#include <stdio.h>
#ifdef _DEBUG
    #include "assert.h"
#else
    #ifndef assert
        #define assert(expr) (void)(expr);
    #endif
#endif

#if defined(_WINDOWS) || defined(XBOX)
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