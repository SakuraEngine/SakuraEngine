#pragma once
#include <stdio.h>
#ifdef _DEBUG
    #include "assert.h"
#else
    #define assert(expr) (void)(expr);
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